// protocol.hh - source file for the mailfilter program
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

#ifndef PROTOCOL_HH
#define PROTOCOL_HH

#include "connection.hh"

// This is useful to specify a combination of protocol and encryption,
// e.g. PROTOCOL_POP3 | SSL.
#define PROTOCOL_POP3       2
#define PROTOCOL_APOP       4
#define SSL_C               4096

using namespace std;

class Protocol
{
protected:
  Connection*  conn;
  unsigned int prot_ident;
  unsigned int connect_type;

public:
  virtual                  ~Protocol      (void) { };
  virtual bool             login          (const char* usr,
					   const char* pass,
					   const unsigned int)     const = 0;
  virtual bool             logout         (void)                   const = 0;
  virtual int              remove_msg     (const unsigned int num) const = 0;
  virtual int              status         (void)                   const = 0;
  virtual int              scan           (void)                   const = 0;
          void             set_connection (Connection*);
          void             set_ident      (unsigned int);
          unsigned int     ident          (void)                   const;
};

#endif
