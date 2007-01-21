// rcfile.hh - source file for the mailfilter program
// Copyright (c) 2000 - 2005  Andreas Bauer <baueran@in.tum.de>
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

#ifndef RCFILE_HH
#define RCFILE_HH

// This is necessary to use multiple lexer classes.  See the flex man
// page for further information.
#undef yyFlexLexer
#define yyFlexLexer rcFlexLexer
#include <FlexLexer.h>
#include <fstream>

using namespace std;

// Note: Bodies of the functions are defined inside rcfile.yy.

class RCParser
{
private:
  istream*     isp;
  ostream*     osp;

public:
  RCParser  (istream* ip = 0, ostream* = 0);
  ~RCParser ();
  void parse (void* = 0);
};

#endif
