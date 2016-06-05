// filter.cc - source file for the mailfilter program
// Copyright (c) 2000 - 2009  Andreas Bauer <baueran@gmail.com>
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

#include <iostream>
#include <string>
extern "C"
{
#include <sys/types.h>
#include <regex.h>
}                                                     
#include "filter.hh"
#include "mailfilter.hh"
#include "preferences.hh"

using namespace std;

Filter :: Filter (void)
{ compiled = false; }

Filter :: ~Filter (void)
{ 
  if (compiled)
    regfree (&comp_expr);
}

string Filter :: expression (void) const
{ return expr; }

void Filter :: set_expression (const char* exp)
{ expr = exp; }

int Filter :: compile (void)
{
  int comp_err;

  if ((comp_err = regcomp (&comp_expr, 
			   expr.c_str (),
			   Preferences :: Instance ().reg_type () | ccase ())) 
      != 0)
    {
      try 
	{
	  char* err_buf = new char[regerror (comp_err,
					     &comp_expr,
					     (char*)NULL,
					     (size_t)0)
				   + 1];
	  regerror (comp_err, &comp_expr, err_buf, sizeof *err_buf);
	  ERROR_MSG(err_buf);
	  delete[] (err_buf);
	}
      catch (...) {  throw;  }
    }
  else
    compiled = true;
  
  return comp_err;
}

void Filter :: set_negativity (bool t)
{ negativity = t; }

bool Filter :: is_negative (void) const
{ return negativity; }

int Filter :: ccase (void) const
{ return case_sensitivity; }

void Filter :: set_case (int c)
{ case_sensitivity = c; }

const regex_t* Filter :: comp_exp (void) const
{ return &comp_expr; }
