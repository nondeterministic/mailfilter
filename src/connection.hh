// connection.hh - source file for the mailfilter program
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

#ifndef CONNECTION_HH
#define CONNECTION_HH

#include <string>

// Right now this class is only a dummy, but later it can be useful to
// derive, e.g. a Windoze Socks connection class from it.

using namespace std;

class Connection
{
public:
  virtual               ~Connection   (void) { }
  virtual int           c_open        (const char* host_name,
			 	       int port,
				       int time_out,
				       int protocol) = 0;
  virtual int           c_close       (void) const = 0;
  virtual int           c_read        (bool = false) = 0;
  virtual int           c_write       (const char* msg) = 0;
  virtual const string* c_reply       (void) const = 0;
};

#endif
