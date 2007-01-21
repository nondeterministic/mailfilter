// filter.hh - source file for the mailfilter program
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

#ifndef FILTER_HH
#define FILTER_HH

#include <string>
extern "C" {
#include <regex.h>
#include <sys/types.h> 
}

// Filter modes
#define CASE_DEFAULT       REG_ICASE
#define CASE_SENSITIVE     0
#define CASE_INSENSITIVE   REG_ICASE

using namespace std;

class Filter
{
private:
  string         expr;
  regex_t        comp_expr;
  regex_t        comp_normal_expr;
  // Values can be CASE_SENSITIVE, CASE_INSENSITIVE, or CASE_DEFAULT:
  int            case_sensitivity;
  bool           negativity;
  bool           compiled;

public:
                 Filter          (void);
                 ~Filter         (void);
  string         expression      (void) const;
  void           set_expression  (const char*);
  int            compile         (void);
  void           set_negativity  (bool);
  bool           is_negative     (void) const;
  int            ccase           (void) const;
  void           set_case        (int);
  const regex_t* comp_exp        (void) const;
};

#endif
