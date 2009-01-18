// weeder.hh - source file for the mailfilter program
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

#ifndef WEEDER_HH
#define WEEDER_HH

#include <vector>
#include <string>
#include "header.hh"

using namespace std;

class Weeder
{
private:
  vector<string> msg_ids;
  int            check_duplicates      (Header*);
  int            check_maxlength       (Header*) const;
  int            check_allow_rules     (Header*) const;
  int            check_deny_rules      (Header*) const;
  int            check_scores          (Header*) const;

public:
  int            is_weed               (Header*);
};

#endif
