// header.cc - source file for the mailfilter program
// Copyright (c) 2003 - 2009  Andreas Bauer <baueran@gmail.com>
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
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "header.hh"
#include "preferences.hh"
#include "mailfilter.hh"
#include "defines.hh"
#include "feedback.hh"
extern "C"
{
#include <ctype.h>
}

using namespace std;

extern int cmp_no_case (const string&, const string&);

vector<entry>* Header :: entries (void)
{ return &msg_entries; }

// Taken from mutt in response to APOP security vulnerability.
/* incomplete. Only used to thwart the APOP MD5 attack (#2846). */

int Header :: rfc822_valid_msgid (const char* msgid)
{
  /* msg-id         = "<" addr-spec ">"
   * addr-spec      = local-part "@" domain
   * local-part     = word *("." word)
   * word           = atom / quoted-string
   * atom           = 1*<any CHAR except specials, SPACE and CTLs>
   * CHAR           = ( 0.-127. )
   * specials       = "(" / ")" / "<" / ">" / "@"
                    / "," / ";" / ":" / "\" / <">
		    / "." / "[" / "]"
   * SPACE          = ( 32. )
   * CTLS           = ( 0.-31., 127.)
   * quoted-string  = <"> *(qtext/quoted-pair) <">
   * qtext          = <any CHAR except <">, "\" and CR>
   * CR             = ( 13. )
   * quoted-pair    = "\" CHAR
   * domain         = sub-domain *("." sub-domain)
   * sub-domain     = domain-ref / domain-literal
   * domain-ref     = atom
   * domain-literal = "[" *(dtext / quoted-pair) "]"
   */

  unsigned int l, i;

  if (!msgid || !*msgid)
    return -1;

  l = strlen (msgid);
  if (l < 5) /* <atom@atom> */
    return -1;
  if (msgid[0] != '<' || msgid[l-1] != '>')
    return -1;
  if (!strrchr (msgid, '@'))
    return -1;

  /* TODO: complete parser */
  for (i = 0; i < l; i++)
    if (msgid[i] > 127)
      return -1;

  return 0;
}

void Header :: add_entry (const char* tag, const char* body)
{
  struct entry tmp_entry;
  tmp_entry.tag  = tag;
  tmp_entry.body = body;
  msg_entries.push_back (tmp_entry);

  if (cmp_no_case (tag, "Message-Id") == 0)
    {
      if (!Preferences :: Instance ().ignore_time_stamp() 
	  && rfc822_valid_msgid (body) < 0)
	{
	  Feedback* logger = Feedback :: Instance ();
	  logger->print_err ("POP timestamp in message-ID invalid.");
	  throw WrongMessageIDException();
	}
      set_ID (body);
      return;
    }
 
  if (cmp_no_case (tag, "From") == 0)
    {
      set_from (body);
      return;
    }

  if (cmp_no_case (tag, "To") == 0)
    {
      set_to (body);
      return;
    }
  
  if (cmp_no_case (tag, "Subject") == 0)
    {
      set_subject (body);
      
      try
	{
	  // set_normal_subject may be throwing out of boundary
	  // exceptions.

	  if (Preferences :: Instance ().normal ())
	    set_normal_subject (body);
	}
      catch (const exception& r_err)
	{
	  ERROR_MSG("Runtime exception occured while parsing rcfile: "
		    + (string)r_err.what ());
	  exit (-1);
	}
      return;
    }

  if (cmp_no_case (tag, "Date") == 0)
    {
      set_date (body);
      return;
    }
}

const string* Header :: ID (void) const
{ return &msg_ID; }

void Header :: set_ID (const char* tid)
{
  msg_ID = tid;
}

const string* Header :: from (void) const
{ return &msg_from; }

void Header :: set_from (const char* tfrom)
{ msg_from = tfrom; }

const string* Header :: to (void) const
{ return &msg_to; }

void Header :: set_to (const char* tto)
{ msg_to = tto; }

const string* Header :: subject (void) const
{ return &msg_subject; }

void Header :: set_subject (const char* tsubject)
{ msg_subject = tsubject; }

const string* Header :: normal_subject (void) const
{ return &msg_normal_subject; }

void Header :: set_normal_subject (string ssubject)
{
  try
    {
      unsigned int i = 0;
      while (i < ssubject.length ())
	{
	  // Delete multiple spaces.
	  while (isspace (ssubject[i])
		 && isspace (ssubject[i+1])
		 && i < ssubject.length ())
	    ssubject.erase (i, 1);
	  
	  // Delete all non-alphanumeric characters, except spaces and '@'.
	  if (!isalpha (ssubject[i]) 
	      && !isspace (ssubject[i]) 
	      && ssubject[i] != '@'
	      && !isdigit (ssubject[i]))
	    {
	      ssubject.erase (i, 1);
	      i -= (i > 0 ? 1 : 0);
	    }
	  else
	    i++;
	}
      msg_normal_subject = "Subject: " + ssubject;
    }
  catch (...)
    {
      // Out-of-Range-Exception could be thrown.
      throw;
    }
}

const string* Header :: date (void) const
{ return &msg_date; }

void Header :: set_date (const char* tdate)
{ msg_date = tdate; }

int Header :: size (void) const
{ return msg_size; }

void Header :: set_size (int tsize)
{ msg_size = tsize; }
