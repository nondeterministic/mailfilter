// account.hh - source file for the mailfilter program
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

#ifndef ACCOUNT_HH
#define ACCOUNT_HH

#include <string>
#include <vector>
#include "defines.hh"
#include "protocol.hh"
#include "pop3.hh"
#include "apop.hh"
#include "connection.hh"

using namespace std;

class Account
{
protected:
  string         serv;
  string         user;
  string         pass;
  int            the_port;
  vector<string> msg_ids;
  Protocol*      proto;
  Connection*    conn;

public:
  void           clear           (void);
  string         server          (void);
  void           set_server      (const char*);
  string         usr             (void);
  void           set_usr         (const char*);
  string         passwd          (void);
  void           set_passwd      (const char*);
  unsigned int   port            (void);
  void           set_port        (unsigned int);
  void           set_protocol    (unsigned int);
  unsigned int   protocol        (void);
  void           set_connection  (unsigned int = POSIX_SOCKETS)
                                  __attribute__ ((unused));
  int            check           (void);
};

#endif
