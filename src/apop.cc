// apop.cc - source file for the mailfilter program
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

#include <cstdio>
#include <cstring>
#include "apop.hh"
#include "feedback.hh"
#include "defines.hh"
#include "mailfilter.hh"

extern "C"
{
#include <strings.h>
#include "md5.h"
}

using namespace std;

bool APOP :: login (const char* usr, 
		    const char* pass, 
		    const unsigned int enc) const
{
  Feedback* logger = Feedback :: Instance ();
  string usr_name = (string)"USER " + usr + (string)"\r\n";
  string pass_wd = (string)"PASS " + pass + (string)"\r\n";
  string greet, command;
  char *ts,*p, md5hash[33];

  // Get server welcome string first.
  if (conn->c_read () == -1)
    return false;

  // Attempt to extract the timestamp from the saved input.
  // If the server response does not include <timestamp>,
  // i.e. the comparison with NULL, it does not support the
  // APOP protocol.
  greet = *conn->c_reply ();
  if ( ((ts = index (&greet[0], '<')) == NULL)
       || ((p = index (ts, '>')) == NULL) )
    return false;
  p[1] = '\0';
  
  // Calculate the hash.
  get_hash (md5hash, ts, pass);
  
  // Send the APOP command as in the username/password code below.
  command = "APOP " + (string)usr + " " + (string)md5hash + "\r\n";
  if (conn->c_write (command.c_str ()) == -1 || !REPLY_OK)
    {
      logger->print_err ("APOP protocol initialisation error.");
      return false;
    }
  
  return true;
}

void APOP :: get_hash (char* hash, const char* stamp, const char* pass) const
{
  MD5_CTX       mdContext;
  unsigned char digest[16];
  
  MD5Init    (&mdContext);
  MD5Update  (&mdContext, (unsigned char*)stamp, strlen (stamp));
  MD5Update  (&mdContext, (unsigned char*)pass, strlen (pass));
  MD5Final   (digest, &mdContext);
  
  for (unsigned int i = 0; i < sizeof (digest); i++)
    sprintf (hash + 2 * i, "%02x", digest[i]);
}
