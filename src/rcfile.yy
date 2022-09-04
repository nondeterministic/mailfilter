/* *********************************************************************** */
/* A parser definition for Mailfilter's config files                       */
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
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#ifdef yyalloc
#define rcalloc_ALREADY_DEFINED
#else
#define yyalloc rcalloc
#endif

#ifdef yysymbol_kind_t
#define rcsymbol_kind_t_ALREADY_DEFINED
#else
#define yysymbol_kind_t rcsymbol_kind_t
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mailfilter.hh"
#include "preferences.hh"
#include "rcfile.hh"

// We want to give the Preferences object as parameter for yyparse.
// #undef YYPARSE_PARAM
// #define YYPARSE_PARAM param

// Define P_DEBUG_MODE to get extra output from the parser.
#ifdef P_DEBUG_MODE
#undef P_DEBUG_MSG
#define P_DEBUG_MSG(msg)    \
   cout << "Parser: "       \
        << msg              \
        << endl             \
        << endl
#else
#define P_DEBUG_MSG(msg)    \
   ;
#endif

using namespace std;

FlexLexer*          rclexer;
extern int          num_lines;
extern string       sub_file;

extern string       int_to_string (int);
extern string       exec_shell  (const char*);
       void         strip_shell (char[]);

inline int yylex()
{  
  return rclexer->rclex();
}

extern "C"
{
  int rcparse(void*);
  // int rcparse(const char*);

  int rcwrap(void)
  {
    return 1;
  }

  void rcerror (const void* str, const void* NOT_USED)
  {
    cerr << PACKAGE_NAME << ": Error: " << str;
    if (sub_file.length ())
      cerr << " in file " << sub_file;
    else
      cerr << " in main rcfile";
    cerr << " in line " << (num_lines + 1);
    cerr << "." << endl;
    exit (-1);
  }
}

%}

%union rc
{
  int   ival;
  char* sval;
}

%parse-param {void* param}

%token <sval> ALLOW
%token <sval> ALLOW_CASE
%token <sval> ALLOW_NOCASE
%token <sval> DEL_DUPLICATES
%token <sval> DENY_NOCASE
%token <sval> DENY_CASE
%token <sval> DENY
%token <sval> HIGHSCORE
%token <sval> LOGFILE
%token <sval> MAXLENGTH
%token <sval> MAXSIZE_ALLOW
%token <sval> MAXSIZE_DENY
%token <sval> MAXSIZE_SCORE
%token <sval> NORMAL
%token <sval> SERVER
%token <sval> USER
%token <sval> PASS
%token <sval> PROTOCOL
%token <sval> PORT
%token <sval> REG_CASE
%token <sval> REG_TYPE
%token <sval> SHOW_HEADERS
%token <sval> SCORE
%token <sval> SCORE_CASE
%token <sval> SCORE_NOCASE
%token <sval> TIMEOUT
%token <sval> TEST
%token <sval> VERBOSE
%token <sval> EXP
%token <sval> YES_NO_ID
%token <sval> TEXT_ID
%token <ival> NUM_ID
%token <sval> SHELL_CMD
%token <sval> ENV_VAR
%token <sval> CTRL_CHAR

%type <sval> exp
%type <sval> exps
%type <sval> str_arg
%type <sval> yes_no_arg
%type <ival> num_arg

%left <sval> NOT_EQUAL
%left <sval> '='

%%
commands: 	/* empty */
        	| commands command
        	;

command:	allow_rule
		| del_duplicates
		| deny_rule
		| highscore
		| logfile
		| maxlength
		| maxsize_allow
		| maxsize_deny
		| maxsize_score
		| normal
		| server
		| user
		| pass
		| protocol
		| port
		| reg_case
		| reg_type
		| show_headers
		| score
		| test
		| timeout
		| verbose
		;

exps:		exp
		{ $$ = $1; }
		| exps exp
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
		      cerr << "Out of memory error in rcparser." << endl;
		      exit (-1);
                    }
		}
		;

exp:		SHELL_CMD
		{ strip_shell ($1);
		  $$ = strdup (exec_shell ($1).c_str ());
		  free ($1);
		}
		| ENV_VAR         { $$ = $1; }
		| TEXT_ID         { $$ = $1; }
		| CTRL_CHAR       { $$ = $1; }
		;

str_arg:	'"' exps '"'      { $$ = $2; }
		;

yes_no_arg:     '"' YES_NO_ID '"' { $$ = $2; }
		;

num_arg:	NUM_ID            { $$ = $1; }
		;

allow_rule:
		ALLOW '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_allow_rule ($1, $2, $3); 
		  free ($1); free ($2); free ($3); }
		| ALLOW NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_allow_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| ALLOW_CASE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_allow_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| ALLOW_CASE NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
	          Preferences :: Instance ().add_allow_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| ALLOW_NOCASE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_allow_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| ALLOW_NOCASE NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_allow_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		;

del_duplicates:
		DEL_DUPLICATES '=' yes_no_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_del_duplicates ($3);
		  free ($1); free ($2); free ($3); }
		;

deny_rule:
		DENY '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
 		  free ($1); free ($2); free ($3); }
		| DENY NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| DENY_CASE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| DENY_CASE NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| DENY_NOCASE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		| DENY_NOCASE NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().add_deny_rule ($1, $2, $3);
		  free ($1); free ($2); free ($3); }
		;

highscore:
		HIGHSCORE '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_highscore ($3);
		  free ($1); free ($2); }
		;

logfile:
		LOGFILE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().set_log_file ($3);
		  free ($1); free ($2); free ($3);  }
		;

maxlength:
		MAXLENGTH '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_maxlength ($3);
		  free ($1); free ($2); }
		;

maxsize_allow:
		MAXSIZE_ALLOW '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_allow ($3);
		  free ($1); free ($2); }
		;

maxsize_deny:
		MAXSIZE_DENY '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_deny ($3);
		  free ($1); free ($2); }
		;

maxsize_score:
		MAXSIZE_SCORE num_arg '=' num_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
                              + (string)$3 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_max_size_score ($2, $4);
		  free ($1); free ($3); }
		;

normal:
		NORMAL '=' yes_no_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_normal ($3);
		  free ($1); free ($2); free ($3); }
		;

server:
		SERVER '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_server ($3);
		  free ($1); free ($2); free ($3); }
		;

user:
		USER '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_usr ($3);
		  free ($1); free ($2); free ($3); }
		;

pass:
		PASS '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_passwd ($3);
		  free ($1); free ($2); free ($3); }
		;

protocol:
		PROTOCOL '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_protocol ($3);

                  /* See account.cc for further information about
                     the following (rather misplaced) call:  */

                  Preferences :: Instance ().set_connection ();
		  free ($1); free ($2); free ($3); }
		;

port:
		PORT '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_port ((unsigned int)$3);
		  free ($1); free ($2); }
		;

reg_case:
		REG_CASE '=' yes_no_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_default_case ($3);
		  free ($1); free ($2); free ($3); }
		;

reg_type:
		REG_TYPE '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_reg_type ($3);
		  free ($1); free ($2); free ($3); }
		;

show_headers:
		SHOW_HEADERS '=' str_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
                  Preferences :: Instance ().set_headers_file ($3);
		  free ($1); free ($2); free ($3);  }
		;

score:
		SCORE num_arg '=' str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
                              + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		| SCORE num_arg NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
	                      + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		| SCORE_CASE num_arg '=' str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
			      + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		| SCORE_CASE num_arg NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
			      + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		| SCORE_NOCASE num_arg '=' str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
			      + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		| SCORE_NOCASE num_arg NOT_EQUAL str_arg
		{ P_DEBUG_MSG($1 + int_to_string (rclval.ival)
			      + (string)$3 + $4);
		  Preferences :: Instance ().add_score ($1, $2, $3, $4);
		  free ($1); free ($3); free ($4); }
		;

test:		TEST '=' yes_no_arg
		{ P_DEBUG_MSG($1 + (string)$2 + $3);
		  Preferences :: Instance ().set_test_mode ($3);
		  free ($1); free ($2); free ($3); }
		;

timeout:
		TIMEOUT '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_time_out ((unsigned int)$3);
		  free ($1); free ($2); }
		;

verbose:
		VERBOSE '=' num_arg
		{ P_DEBUG_MSG($1 + (string)$2 + int_to_string (rclval.ival));
		  Preferences :: Instance ().set_verbose_level ($3);
		  free ($1); free ($2); }
		;
%%

/* This function strips the leading, and trailing quotation marks
   around a shell `command' to be executed by the POSIX shell.  */

void strip_shell(char s[])
{
  string buf = s;
  for (unsigned int i = 1; i < buf.length () - 1; i++) {
    s[i-1] = buf[i];
    s[i] = '\0';
  }
}

/* The class declarations can be found in rcfile.hh.  */

RCParser :: RCParser(istream* ip, ostream* op)
  : isp(ip), osp(op)
{
  try {
    rclexer = new rcFlexLexer(isp, osp);
  }
  catch (...) {  
    throw;  
  } 
}

RCParser :: ~RCParser()
{ 
  delete rclexer;
}

void RCParser :: parse(void* val)
{
  rclexer->yyrestart(isp);
  rcparse(val);
}
