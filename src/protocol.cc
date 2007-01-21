// protocol.cc - source file for the mailfilter program
// Copyright (c) 2003  Andreas Bauer <baueran@in.tum.de>
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

#include "connection.hh"
#include "protocol.hh"

using namespace std;

void Protocol :: set_ident (unsigned int i)
{ prot_ident = i; }

unsigned int Protocol :: ident (void) const
{ return prot_ident; }

void Protocol :: set_connection (Connection* currently_established_connection)
{
  conn = currently_established_connection;
}
