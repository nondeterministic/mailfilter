// socket.hh - source file for the mailfilter program
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

#ifndef SOCKET_HH
#define SOCKET_HH

#include <string>
#include <csignal>

#if USE_SSL
extern "C"
{
#include <openssl/ssl.h>
#include <openssl/rand.h>
}
#endif

#include "connection.hh"

using namespace std;

#ifndef MAX_BYTES
#define MAX_BYTES 512
#endif

class Socket : public Connection
{
private:
  int               sd;                     // Socket descriptor.
  int               time_out;               // Time out.
  static void       connect_alarm (int);    // Alarm handler.
  string*           read_buffer;            // Server replies after read ().
  bool              ssl_used;               // True if SSL encryption is used.
  bool              use_ssl       (void) const;
  void              set_ssl       (bool);

public:
                    Socket        (void);
  void              clear         (void);
  int               c_open        (const char* host,
			           int port,
           		           int time_out,
			           int protocol);
  int               c_close       (void) const;
  int               c_write       (const char* command);
  int               c_read        (bool = false);
  const string*     c_reply       (void) const;
};

#endif
