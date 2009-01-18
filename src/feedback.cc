// feedback.cc - source file for the mailfilter program
// Copyright (c) 2000 - 2009  Andreas Bauer <baueran@gmail.com>
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

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "feedback.hh"
#include "preferences.hh"

using namespace std;

Feedback* Feedback :: _instance = 0;

Feedback* Feedback :: Instance ()
{
  if (_instance == 0)
    _instance = new Feedback;
  return _instance;
}

Feedback :: ~Feedback ()
{ 
  log_file.close ();
  if (header_file.is_open ())
    header_file.close ();
}

// This function tries to open a user specified log file for write
// access and returns true upon success, false otherwise.

bool Feedback :: open (const char* name)
{
  if (strlen (name))
    {
      log_file.open (name, ios :: app);

      if (!log_file.is_open ())
	return false;

      return true;
    }
    
  return false;
}

// The following two functions attempt to append program messages to
// the end of the log file and returns false, if the file is not
// accessable.  True is returned otherwise.
// 
// min_verbose_level is the minimum value of verbosity that is
// necessary, in order to display a message.  If nothing is
// specified, errors are usually always shown (0).

bool Feedback :: print_msg (const string msg, int min_verbose_level)
{
  if (Preferences :: Instance ().verbose_level () >= min_verbose_level)
    {
      cout << "mailfilter: " << msg << endl;
	
      if (log_file.is_open ())
	log_file << "mailfilter: " << msg << endl;
      else
	return false;
    }

  return true;
}
  
bool Feedback :: print_err (const string msg, int min_verbose_level)
{
  if (Preferences :: Instance ().verbose_level () >= min_verbose_level)
    {
      cerr << "mailfilter: Error: " << msg << endl;
	
      if (log_file.is_open ())
	log_file << "mailfilter: Error: " << msg << endl;
      else
	return false;
    }
    
  return true;
}

bool Feedback :: print_header (const string msg)
{
  if (!header_file.is_open ())
    header_file.open (Preferences :: Instance ().headers_file ().c_str (), ios :: app);
  
  if (header_file.is_open ())
    {
      header_file << msg << endl;
      return true;
    }
  
  return false;
}
