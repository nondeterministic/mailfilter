// pop3.cc - source file for the mailfilter program
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

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include "socket.hh"
#include "pop3.hh"
#include "feedback.hh"
#include "preferences.hh"
#include "mailfilter.hh"
#include "header.hh"
#include "weeder.hh"
#include "defines.hh"
#include "protocol.hh"

// This is necessary to use multiple lexer classes.  See the flex man
// page for further information.
#undef yyFlexLexer
#define yyFlexLexer rfcFlexLexer
#include <FlexLexer.h>

extern "C"
{ int rfcparse (void*); }

using namespace std;

// Declare lexer globally, so the parser can find it.
FlexLexer*          rfclexer;
extern Weeder       weeder;

bool POP3 :: login (const char* usr, 
		    const char* pass, 
		    const unsigned int enc) const
{
  string usr_name = (string)"USER " + usr + (string)"\r\n";
  string pass_wd = (string)"PASS " + pass + (string)"\r\n";

  if (conn->c_read () == -1)
    return false;

  Feedback* logger = Feedback :: Instance ();
  
  // Send user name and read server reply.
  if (conn->c_write (usr_name.c_str ()) == -1 || !REPLY_OK)
    {
      logger->print_err("Error occured while sending username to server.");
      return false;
    }

  // Send password and read server reply.
  if (conn->c_write (pass_wd.c_str ()) == -1 || !REPLY_OK)
    {
      logger->print_err("Error occured while sending password to server.");
      return false;
    }

  return true;
}

int POP3 :: status (void) const
{
  // Send user name and read server reply.
  if (conn->c_write ("STAT\r\n") == -1)
    return GEN_FAILURE_FLAG;
  if (!REPLY_OK)
    return GEN_FAILURE_FLAG;

  istringstream response (conn->c_reply ()->c_str ());
  string no_msgs;

  // The second word in the server's output string contains the number
  // of unread messages in a POP3 mailbox.  Hence we shift the reply
  // string twice.
  for (int i = 1; i <= 2; i++)
    response >> no_msgs;
  return atoi (no_msgs.c_str ());
}

// The function scans the headers inside a POP3 account for spam.  It
// will delete all spam messages in the account and return 0 when all
// the hard work is done.  A negative integer is returned if an error
// occured.

int POP3 :: scan (void) const
{
  Feedback* logger = Feedback :: Instance ();

  Header* msg_header;
  int num_messages;
  stringstream msg_no;
  string cmd;

  // Determine number of messages waiting to be examined.
  if ((num_messages = status ()) < 0)
    {
      logger->print_err ("Error occured while sending STAT to server.");
      return GEN_FAILURE_FLAG;
    }
  
  try
    {
      for (int i = 1; i <= num_messages; i++)
	{
	  // Reserve heap for the message to be stored, parsed, and
	  // processed.
	  msg_header = new Header;
	  
	  // Convert current message number to string.
	  msg_no << i;
	  
	  // Determine message size.
	  cmd = "LIST " + msg_no.str () + "\r\n";
	  if (conn->c_write (cmd.c_str ()) == -1 || !REPLY_OK)
	    {
	      logger->print_err ("Error occured while sending LIST to server.");
	      return GEN_FAILURE_FLAG;
	    }
	  
	  msg_header->set_size (atoi
				((conn->c_reply ()->
				  substr (conn->c_reply ()->
					  find_last_of (" ") + 1)).c_str ()));
	  
	  // Read the header of the current message.
	  cmd = "TOP " + msg_no.str () + " 0\r\n";
	  if (conn->c_write (cmd.c_str ()) == -1 || !HEADER_OK)
	    {
	      logger->print_err ("Error occured while sending TOP to server.");
	      return GEN_FAILURE_FLAG;
	    }

	  // Store the header in a separate file, if the user has given
	  // a path definition via SHOW_HEADERS.
	  if (Preferences :: Instance ().headers_file ().length ())
	    if (!logger->print_header (conn->c_reply ()->c_str ()))
	      logger->print_err ("Could not write headers to separate file.");
	  
	  // Strip topmost status line of server reply, e.g.  "+OK Message
	  // follows."  The +1 in the end is necessary to skip the actual
	  // first newline itself.
	  string message = conn->c_reply ()->substr
	    (conn->c_reply ()->find_first_of ("\n") + 1);
	  
	  // Now parse the header of the current message and store it in
	  // the msg_header object.
	  if (invoke_msg_parser (&message, msg_header) < 0)
	    {
	      logger->print_err ("Parsing the header of message "
				+ msg_no.str ()
				+ " failed.");
	      return GEN_FAILURE_FLAG;
	    }
	  
	  // Now pass msg_header on to the weeder in order to determine
	  // whether it stores a spam mail.
	  if (weeder.is_weed (msg_header) == 1)
	    remove_msg (i);

	  // Delete memory occupied by the current message header.
	  delete msg_header;
	  
	  // Reset stringstream for next int to string conversion.
	  msg_no.clear ();
	  msg_no.str (string ());
	}
    }
  catch (...) { throw; }

  return 0;
}

// This function accepts a string pointer as argument which contains
// the entire header of an email message.  It is used to parse that
// header and store it in a Header-class, msg_header, in order to
// determine whether that particular message qualifies as Spam.  The
// function returns a positive integer upon success, and a negative
// one otherwise.  Failure is usually related to out-of-memory errors.

int POP3 :: invoke_msg_parser (const string* header, 
			       const Header* msg_header) const
{
  if (header && msg_header)
    {
      try
	{
	  stringstream cur_header;
	  cur_header << *header;
	  rfclexer = new rfcFlexLexer;
	  rfclexer->switch_streams (&cur_header);
	  int error = rfcparse ((void*) msg_header);
	  delete rfclexer;
	  return error;
	}
      catch (...) {  return MEM_FAILURE_FLAG; }
    }
  else
    return GEN_FAILURE_FLAG;
}

bool POP3 :: logout (void) const
{ return (conn->c_write ("QUIT\r\n") == -1) ? false : true; }

int POP3 :: remove_msg (const unsigned int num) const
{  
  if (Preferences :: Instance ().test_mode ())
    {
      Feedback* logger = Feedback :: Instance ();
      logger->print_msg ("Debugging: Simulating DELE command.", 6);
      return 0;
    }

  ostringstream ostr; ostr << num;
  string cmd = (string)"DELE " + ostr.str () + (string)"\r\n";
  return (conn->c_write (cmd.c_str ()) == -1) ? GEN_FAILURE_FLAG : 0;
}
