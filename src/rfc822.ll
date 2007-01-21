/* *********************************************************************** */
/* An oversimplified scanner definition for Mailfilter's RFC822 scanner    */
/* Copyright (c) 2000 - 2005  Andreas Bauer <baueran@in.tum.de>            */
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

%x BODY_READY
%s TAG_READY
%option noyywrap

%{
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "rfc822parser.hh"

using namespace std;

/* Define L_DEBUG_MODE to get extra output from the scanner
   and to avoid returning too early.  Early return is useless
   if there is no parser which keeps calling the scanners.  */

#undef L_DEBUG_MODE

/* Return should be defined to a nop-operation, if the programmer
   really only debugs a single message offline.  */

#define RETURN(x) return(x)

#ifdef  L_DEBUG_MODE
#undef  L_DEBUG_MSG
#define L_DEBUG_MSG(msg)    \
   cout << "Lexer: "        \
        << msg
#else
#undef  L_DEBUG_MSG
#define L_DEBUG_MSG(msg)    \
   ;
#endif

%}

NL              (\r\n)
TOPLINE         ^From[[:space:]].*
TAG             ^[[:alpha:]]+([[:alnum:]]|\-)*\:
BODY            (.*|[[:space:]])
HEADER_END      \.

%%
<INITIAL>{TOPLINE}{NL} {
	rfclval.sval = strdup (yytext);
	L_DEBUG_MSG("Topline: " + (string)yytext);
	RETURN(TOPLINE);
}

<INITIAL,TAG_READY>{TAG}[[:space:]]* {
	string the_tag = yytext;
	the_tag = the_tag.substr (0, the_tag.find_last_of (":"));
	rfclval.sval = strdup (the_tag.c_str ());
	L_DEBUG_MSG("   Tag: " +  the_tag + "\n");
	BEGIN(BODY_READY);
	RETURN(TAG);
}

<BODY_READY>{BODY}{NL} {
	/* Remove trailing newline if it exists.  */
	if (yytext[strlen(yytext) - 1] == '\n')
	  {
	    char* the_tag = (char*)malloc (sizeof (char) * strlen (yytext));
	    snprintf (the_tag, strlen (yytext) - 1, "%s", yytext);
	    rfclval.sval = the_tag;
	  }
	else
	  rfclval.sval = strdup (yytext);
	L_DEBUG_MSG("  Body: " + (string)yytext);
	BEGIN(0);
	RETURN(BODY);
}

^[[:space:]].* {
	/* This is to scan multiple lines of a header
	   body field, e.g. the Received fields are
	   usually spread over multiple lines.
	   In order to handle the leading white spaces,
	   we need to replace them with ordinary blanks.  */
	char* the_tag = yytext;
	for (unsigned int i = 0; i <= strlen (the_tag); i++)
	   if (isspace (the_tag[i])) the_tag[i] = ' ';
	rfclval.sval = strdup (the_tag);
	L_DEBUG_MSG("M-Body: " + (string)the_tag);
	BEGIN(TAG_READY);
	RETURN(BODY);
}

{NL}{HEADER_END}{NL} {
	rfclval.sval = strdup (yytext);
	L_DEBUG_MSG("EOF.\n");
	RETURN(HEADER_END);
}

<*>[ \t] {; /* Do nothing with tabs and spaces.  */ }

<*>"\n" { BEGIN(0); }

%%
/* In order to test the following code and to create a standalone scanner,
   read the comments inside Makefile.am and compile only the Makefile
   target `rfc_test'.  */

int rfcwrap ();

#ifdef SCANNER_STANDALONE
#include <fstream>

int main (int argc, char** argv)
{
    FlexLexer* lexer = new rfcFlexLexer;
    ++argv, --argc;  /* Skip over program name.  */
    lexer->switch_streams (new ifstream (argv[0]));
    lexer->rfclex ();
    delete lexer;
    return 0;
}
#endif

int rfcwrap () { return 1; }
