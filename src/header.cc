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
#include <stdexcept>
#include "header.hh"
#include "preferences.hh"
#include "mailfilter.hh"
#include "defines.hh"
extern "C"
{
#include <ctype.h>
}

using namespace std;

extern int cmp_no_case (const string&, const string&);

vector<entry>* Header :: entries (void)
{ return &msg_entries; }

void Header :: add_entry (const char* tag, const char* body)
{
  struct entry tmp_entry;
  tmp_entry.tag  = tag;
  tmp_entry.body = body;
  msg_entries.push_back (tmp_entry);

  if (cmp_no_case (tag, "Message-Id") == 0)
    {
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
{ msg_ID = tid; }

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
