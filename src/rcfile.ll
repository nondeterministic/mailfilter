/* *********************************************************************** */
/* A scanner definition for Mailfilter's config files                      */
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

/* The states are numbered from 1...n and are stored in YY_START.  */
%s CTRL
%x INCL
%x PARAM
%s PARAM_NUM
%s PARAM_BOOL

%option noyywrap

%{
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rcparser.hh"
#include "i18n.hh"

extern "C"
{
#include <wordexp.h>
}

using namespace std;

/* Only allow 'recursion depth' of 1 when including files.  */
#define MAX_INCLUDE_DEPTH   1

/* Define L_DEBUG_MODE to get extra output from the scanner.  */
#ifdef L_DEBUG_MODE
#undef L_DEBUG_MSG
#define L_DEBUG_MSG(msg)    \
   cout << "Lexer: "        \
        << msg              \
        << endl
#else
#define L_DEBUG_MSG(msg)    \
   ;
#endif

YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int             include_stack_ptr;

extern string expand_path (const string&);
extern string int_to_string (int val);
%}

ALLOW           allow
ALLOW_CASE      allow_case
ALLOW_NOCASE    allow_nocase
DEL_DUPLICATES  del_duplicates
DENY_NOCASE     deny_nocase
DENY_CASE       deny_case
DENY            deny
HIGHSCORE       highscore
INCLUDE         include[ \t]*=
LOGFILE         logfile
MAXLENGTH       maxlength
MAXSIZE_ALLOW   maxsize_allow
MAXSIZE_DENY    maxsize_deny
MAXSIZE_SCORE   maxsize_score
NORMAL          normal
REG_CASE        reg_case
REG_TYPE        reg_type
SERVER          server
USER            user
PASS            pass
PROTOCOL        protocol
PORT            port
SHOW_HEADERS    show_headers
SCORE           score
SCORE_CASE      score_case
SCORE_NOCASE    score_nocase
TEST            test
TIMEOUT         timeout
VERBOSE         verbose
REM		^[[:blank:]]*#.*
NOT_EQUAL	<>
EXP		\"([[:graph:]]+)|([[:graph:]]+.*[[:graph:]]+)\"
TEXT_ID		[[:alnum:]]+
YES_NO_ID       (yes|no)
SHELL_CMD	\`([[:graph:]]|[[:blank:]])+\'
ENV_VAR		[\$]([[:alnum:]]|_)+
NUM_ID		(\+|\-|[0-9])[0-9]*
CTRL_CHAR	.

		int    num_lines      = 0;
		int    temp_num_lines = 0;
		string sub_file;

%%
{INCLUDE} {
     BEGIN(INCL);
     temp_num_lines = num_lines;
     num_lines = 0;
}

<INCL>[ \t]* {; /* Eat the whitespace.  */ }

<INCL>[^ \t\n]+ {
     /* Include further rcfiles:  */
     wordexp_t result;

     if (include_stack_ptr >= MAX_INCLUDE_DEPTH)
     {
        cerr << PACKAGE_NAME << _(": Error: Files nested too deep.") << endl;
	exit (-1);
     }
     include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
     
     try
     {
        if (wordexp (yytext, &result, 0) == 0)
        {
	   sub_file = result.we_wordv[0];
           wordfree (&result);
        }
        else
        {
           cerr << PACKAGE_NAME << _(": Error: Nested rcfile '");
	   cerr << yytext << _("' could not be found.") << endl;
	   exit (-1);
        }

        yyin = new ifstream (sub_file.c_str ());

        if (!((ifstream*) yyin)->is_open ())
        {
           cerr << PACKAGE_NAME << _(": Error: Nested rcfile '");
	   cerr << sub_file << _("' could not be opened.") << endl;
	   exit (-1);
        }

        yy_switch_to_buffer (yy_create_buffer (yyin, YY_BUF_SIZE));
     }
     catch (...)
     {
	cerr << PACKAGE_NAME << _(": Error: Exception was thrown when ")
	     << _("trying to read '")
             << yytext << "'." << endl;
        exit (-1);
     }

     BEGIN(INITIAL);
}

<<EOF>> {
     if (--include_stack_ptr < 0)
        yyterminate ();
     else
     {
        yy_delete_buffer (YY_CURRENT_BUFFER);
        yy_switch_to_buffer (include_stack[include_stack_ptr]);
     }

     num_lines = temp_num_lines;
     sub_file = "";
}

{REM} {; /* Do nothing with remarks.  */ }

{ALLOW} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Allow.");
     return ALLOW;
}

{ALLOW_CASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Allow_Case.");
     return ALLOW_CASE;
}

{ALLOW_NOCASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Allow_Nocase.");
     return ALLOW_NOCASE;
}

{DEL_DUPLICATES} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Del_Duplicates.");
     BEGIN(PARAM_BOOL);
     return DEL_DUPLICATES;
}

{DENY} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Deny.");
     return DENY;
}

{DENY_CASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Deny_Case.");
     return DENY_CASE;
}

{DENY_NOCASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Deny_Nocase.");
     return DENY_NOCASE;
}

{HIGHSCORE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Highscore.");
     BEGIN(PARAM_NUM);
     return HIGHSCORE;
}

{LOGFILE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Logfile.");
     return LOGFILE;
}

{MAXLENGTH} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Maxlength.");
     BEGIN(PARAM_NUM);
     return MAXLENGTH;
}

{MAXSIZE_ALLOW} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Maxsize_Allow.");
     BEGIN(PARAM_NUM);
     return MAXSIZE_ALLOW;
}

{MAXSIZE_DENY} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Maxsize_Deny.");
     BEGIN(PARAM_NUM);
     return MAXSIZE_DENY;
}

{MAXSIZE_SCORE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Maxsize_Score.");
     BEGIN(PARAM_NUM);
     return MAXSIZE_SCORE;
}

{NORMAL} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Normal.");
     BEGIN(PARAM_BOOL);
     return NORMAL;
}

{SERVER} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Server.");
     return SERVER;
}

{USER} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("User.");
     return USER;
}

{PASS} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Pass.");
     return PASS;
}

{PROTOCOL} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Protocol.");
     return PROTOCOL;
}

{PORT} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Port.");
     BEGIN(PARAM_NUM);
     return PORT;
}

{REG_CASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Reg_Case.");
     BEGIN(PARAM_BOOL);
     return REG_CASE;
}

{REG_TYPE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Reg_Type.");
     return REG_TYPE;
}

{SHOW_HEADERS} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Show_Headers.");
     return SHOW_HEADERS;
}

{SCORE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Score.");
     BEGIN(PARAM_NUM);
     return SCORE;
}

{SCORE_CASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Score_Case.");
     BEGIN(PARAM_NUM);
     return SCORE_CASE;
}

{SCORE_NOCASE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Score_Nocase.");
     BEGIN(PARAM_NUM);
     return SCORE_NOCASE;
}

{TEST} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Test.");
     BEGIN(PARAM_BOOL);
     return TEST;
}

{TIMEOUT} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Timeout.");
     BEGIN(PARAM_NUM);
     return TIMEOUT;
}

{VERBOSE} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Verbose.");
     BEGIN(PARAM_NUM);
     return VERBOSE;
}
     
"=" {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG((string)rclval.sval +
          " (" + int_to_string (YY_START) + ")");
     return '=';
}

"<>" {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG((string)rclval.sval +
	         " (" + int_to_string (YY_START) + ")");
     return NOT_EQUAL;
}

"\"" {
     /* Beginning of an argument:  */
     rclval.sval = strdup (yytext);
     if (YY_START != PARAM_BOOL)
       BEGIN(PARAM);
     L_DEBUG_MSG((string)"Opening \"" +
	         " (" + int_to_string (YY_START) + ")");
     return '"';
}

<PARAM_NUM>{NUM_ID} {
     rclval.ival = atoi (yytext);
     L_DEBUG_MSG("Num_Id: " + (string)yytext +
	         " (" + int_to_string (YY_START) + ")");
     return NUM_ID;
}

<PARAM>{SHELL_CMD} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Shell_Cmd: " + (string)rclval.sval +
	         " (" + int_to_string (YY_START) + ")");
     return SHELL_CMD;
}

<PARAM>{ENV_VAR} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Env_Var: " + (string)rclval.sval +
	         " (" + int_to_string (YY_START) + ")");
     return ENV_VAR;
}

<PARAM_BOOL>{YES_NO_ID} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Yes_No_Id: " + (string)rclval.sval + 
	         " (" + int_to_string (YY_START) + ")");
     return YES_NO_ID;
}

<PARAM>{TEXT_ID} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Text_Id: " + (string)rclval.sval +
                 " (" + int_to_string (YY_START) + ")");
     return TEXT_ID;
}

<PARAM,PARAM_BOOL>\"[[:blank:]]*$ {
     /* Ending of an argument:  */
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG((string)"Closing \"" +
	         " (" + int_to_string (YY_START) + ")");
     return '"';
}

<PARAM>{CTRL_CHAR} {
     rclval.sval = strdup (yytext);
     L_DEBUG_MSG("Ctrl_Char: " + (string)rclval.sval +
	         " (" + int_to_string (YY_START) + ")");
     return CTRL_CHAR;
}

<*>[ \t] {; /* Do nothing with tabs and spaces.  */ }

<*>"\n" { ++num_lines; BEGIN(0); }

<*>. {
     if (sub_file != "")
     {
     	cerr << PACKAGE_NAME << _(": Error: Lexicographical error in line ");
        cerr << (num_lines + 1) << _(" of your rcfile '") << sub_file;
        cerr << "'."  << endl;
     }
     else
     {
        cerr << PACKAGE_NAME << _(": Error: Lexicographical error in line ");
        cerr << (num_lines + 1) << _(" of your main rcfile.")  << endl;
     }

     cerr << PACKAGE_NAME << _(": The term '") << yytext;
     cerr << _("' could not be interpreted.") << endl;
     exit (-1);
}

%%
