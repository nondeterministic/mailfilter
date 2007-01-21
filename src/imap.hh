// imap.hh - source file for the mailfilter program
// Copyright (c) 2004  Andreas Bauer <baueran@in.tum.de>
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

#ifndef IMAP_HH
#define IMAP_HH

#include "header.hh"
#include "protocol.hh"

using namespace std;

// True, if the server replied and its status message was anything,
// but an error.
#define REPLY_OK ((conn->c_read () > 0 && conn->c_reply ()) ?            \
        (((conn->c_reply ()->find_first_of ("a OK", 0))) ? true : false) \
                                                                : false)

// This macro is similar to REPLY_OK, except it sets a flag for c_read
// in order to tell the function that an entire message header is
// about to be received.  Further comments inside socket.cc:c_read().
#define HEADER_OK ((conn->c_read (true) > 0 && conn->c_reply ())?        \
        (((conn->c_reply ()->find_first_of ("a OK", 0))) ? true : false) \
                                                                : false)

class IMAP : public Protocol
{
private:
  int      invoke_msg_parser (const string*, const Header*);

public:
  bool     login             (const char* usr,
			      const char* pass,
			      const unsigned int enc);
  bool     logout            (void);
  int      remove_msg        (unsigned int num);
  int      status            (void);
  int      scan              (void);
};

#endif
