/* *********************************************************************** */
/* A parser definition for Mailfilter's RFC 822 parser                     */
/* Copyright (c) 2000 - 2009  Andreas Bauer <baueran@gmail.com>            */
/*                                                                         */
/* This program is free software; you can redistribute it and/or modify    */
/* it under the terms of the GNU General Public License as published by    */
/* the Free Software Foundation; either version 2 of the License, or       */
/* (at your option) any later version.                                     */
/*                                                                         */
/* This program is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/* GNU General Public License for more details.                            */
/*                                                                         */
/* You should have received a copy of the GNU General Public License       */
/* along with this program; if not, write to the Free Software             */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,   */
/* USA.                                                                    */
/* *********************************************************************** */


%{
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include "header.hh"

// This is necessary to use multiple lexer classes.  See the flex man
// page for further information.
#undef yyFlexLexer
#define yyFlexLexer rfcFlexLexer
#include <FlexLexer.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// We want to give the Preferences object as parameter for yyparse
#define YYPARSE_PARAM param

using namespace std;

extern FlexLexer* rfclexer;

inline int yylex ()
{ return rfclexer->rfclex (); }

extern "C"
{
  int rfcparse (void*);

  void rfcerror (const char* str)
  {
    cerr << PACKAGE_NAME << ": Error: Parser reported " << str;
    cerr << "." << endl;
    exit (-1);
  }
}

%}

%union { char* sval; };

%token <sval> HEADER_END
%token <sval> TOPLINE
%token <sval> TAG
%token <sval> BODY
%token <sval> BODY_MULTI_LINE

%type <sval> bodies
%right BODY

%%
header:		/* empty */
		| header entry
		;

entry:		the_end
		| TAG bodies
		  { ((Header*)param)->add_entry ($1, $2);
                    free ($1); free ($2); }
		;

bodies:		BODY  { $$ = $1; }
		| bodies BODY
                  { char* tmp = (char*)malloc ((strlen ($1) + strlen ($2) + 2)
                                                * sizeof (char));
                    if (tmp)
		      {
		        strcpy (tmp, $1);
			$$ = strcat (tmp, $2);
			free ($1); free ($2);
                      }
                    else
                      {
			cerr << "Out of memory error in rfcparser." << endl;
                        exit (-1);
                      }
                  }
		;

the_end:	HEADER_END
		{ ; }
		;

%%
