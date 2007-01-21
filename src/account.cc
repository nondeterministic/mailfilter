// account.cc - source file for the mailfilter program
// Copyright (c) 2003 - 2004  Andreas Bauer <baueran@in.tum.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

#include <string>
#include <vector>
#include <typeinfo>
#include "account.hh"
#include "pop3.hh"
#include "apop.hh"
#include "preferences.hh"
#include "feedback.hh"
#include "mailfilter.hh"
#include "connection.hh"
#include "socket.hh"
#include "defines.hh"

using namespace std;

extern int mailbox_status;
extern string int_to_string (int);

void Account :: clear (void)
{
  if (proto)
    delete proto;

  if (conn)
    {
      if (typeid (*conn) == typeid (Socket))
	((Socket*)conn)->clear ();
      delete conn;
    }
}

string Account :: server (void)
{ return serv; }

void Account :: set_server (const char* s)
{ serv = s; }

string Account :: usr (void)
{ return user; }

void Account :: set_usr (const char* s)
{ user = s; }

string Account :: passwd (void)
{ return pass; }

void Account :: set_passwd (const char* s)
{ pass = s; }

void Account :: set_protocol (unsigned int prot)
{
  try
    {
      switch (prot)
	{
	case PROTOCOL_APOP:
	  proto = new APOP ();
	  proto->set_ident (PROTOCOL_APOP);
	  break;
	case (PROTOCOL_APOP | SSL_C):
	  proto = new APOP ();
	  proto->set_ident (PROTOCOL_APOP | SSL_C);
	  break;
	case (PROTOCOL_POP3 | SSL_C):
	  proto = new POP3 ();
	  proto->set_ident (PROTOCOL_POP3 | SSL_C);
	  break;
	default:
	  proto = new POP3 ();
	  proto->set_ident (PROTOCOL_POP3);
	  break;
	}
    }
  catch (...) {  throw;  }
}

// TODO:
// This function sets the connection type, but currently only
// POSIX Sockets are available.  Therefore, the_connection_type
// gets ignored and a new Socket object is being instantiated
// per default.  Right now, the rcfile parser invokes it together
// with set_protocol (), but once other connection types are
// implemented, it should be handled via a separate keyword and
// action of the parser; not implicitly as it happens now.

void Account :: set_connection (unsigned int the_connection_type)
{
  try
    {
      // Pass a reference to the connection object to the protocol
      // implementing class, so that it can communicate directly with
      // the server e.g. via Unix Sockets.
      conn = new Socket ();
      proto->set_connection (conn);
    }
  catch (...) {  throw;  }
}

unsigned int Account :: protocol (void)
{ return proto->ident (); }

void Account :: set_port (unsigned int p)
{ the_port = p; }

unsigned int Account :: port (void)
{ return the_port; }

int Account :: check (void)
{
  Feedback* logger = Feedback :: Instance ();

  int messages = 0;

  try
    {
      // Open connection to host.
      if (conn->c_open (server ().c_str (),
			port (),
			Preferences :: Instance ().time_out (),
			proto->ident ()) != 0)
	{
	  logger->print_err (_("Could not establish network connection.\n"), 1);
	  return GEN_FAILURE_FLAG;
	}
      
      // Login.
      if (!proto->login (usr ().c_str (),
			 passwd ().c_str (),
			 proto->ident ()))
	{
	  logger->print_err (_("Server login failure.\n"), 1);
	  conn->c_close ();
	  return GEN_FAILURE_FLAG;
	}

      if ((messages = proto->status ()) < 0)
	{
	  logger->print_err (_("Could not determine mailbox status.\n"), 1);
	  proto->logout ();
	  conn->c_close ();
	  return GEN_FAILURE_FLAG;
	}
      
      logger->print_msg (_("Examining "
			  + int_to_string (messages)
			  + " message(s).\n"), 3);
      
      // Scan mailbox for spam and unwanted bulk.
      if (proto->scan () < 0)
	{
	  logger->print_err (_("Scanning of mail account failed.\n"), 1);
	  proto->logout ();
	  conn->c_close ();
	  return GEN_FAILURE_FLAG;
	}

      // Determine number of messages in the mailbox for program
      // return value.
      if (Preferences :: Instance ().return_status ()
	  && (messages = proto->status ()) < 0)
	{
	  logger->print_err (_("Could not determine mailbox status.\n"), 1);
	  proto->logout ();
	  conn->c_close ();
	  return GEN_FAILURE_FLAG;
	}
      else
	mailbox_status += messages;
    }
  catch (...) {  throw;  }

  // Logout and close the connection.
  return ((proto->logout () 
	   && conn->c_close() == 0) ? 0 : GEN_FAILURE_FLAG);
}
