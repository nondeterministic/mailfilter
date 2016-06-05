// mailfilter.cc - source file for the mailfilter program
// Copyright (c) 2000 - 2012  Andreas Bauer <baueran@gmail.com>
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
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <stdexcept>
#include <vector>
#include "mailfilter.hh"
#include "preferences.hh"
#include "feedback.hh"
#include "weeder.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"
{
#include "time.h"
#include <sys/time.h>                                                    
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getopt.h"
#endif
}

using namespace std;

static struct option long_options[] =
  {
    {"help", 0, NULL, VALUE_HELP},
    {"verbose", 1, NULL, VALUE_VERBOSE},
    {"mailfilterrc", 1, NULL, VALUE_MAILFILTERRC},
    {"logfile", 1, NULL, VALUE_LOGFILE},
    {"ignore-time-stamps", 0, NULL, VALUE_TIMESTAMP},
    {"version", 0, NULL, VALUE_VERSION},
    {"test", 0, NULL, VALUE_TEST},
    {"return-value", 0, NULL, VALUE_RETURN},
    {0, 0, 0, 0}
  };

struct sigaction    sigact;
Weeder              weeder;
int                 mailbox_status;

void   init_app               (void);
bool   open_prefs             (string);
void   get_opts               (int argc, char* argv[]);
void   override_prefs         (string);
int    cmp_no_case            (const string&, const string&);
int    precompile_expressions (void);
void   connect_sigint         (int);
string int_to_string          (int);

int main (int argc, char* argv[])
{
  Feedback* logger = Feedback :: Instance ();
  time_t now;
  struct timeval tv;
  struct timezone tz;
  string options_set;
  int    return_val = 0;

  init_app ();

  // Read the user's command line options.
  if (argc > 1)
    get_opts (argc, argv);

  // Try to open the program's configuration file.
  if (!(Preferences :: Instance ().
	open (Preferences :: Instance ().rc_file ().c_str ())))
    {
      ERROR_MSG("The rcfile could not be opened.");
      return -1;
    }

  // If an rcfile was located, try to read it.  The error handling,
  // however, is done inside the Preferences class.  Should something
  // go wrong, we merely need to exit here.  There's no point in
  // logging to external files, since they may not yet be defined, or
  // even be writeable.
  try
    {
      if (!Preferences :: Instance ().load ())
	{
	  ERROR_MSG("Loading the rcfile failed.");
	  return -1;
	}
    }
  catch (const exception& r_err)
    {
      ERROR_MSG("Runtime exception occured: "
		+ (string)r_err.what ());
      return -1;
    }

  // Now that the log file should be specified, try to open it and
  // spit out an error in case it can't be located, or the file
  // permissions don't allow writing to it.
  if (!logger->open (Preferences :: Instance ().log_file ().c_str ()))
  {
    if (Preferences :: Instance ().log_file ().length ())
      ERROR_MSG("Could not open log file '" +
		Preferences :: Instance ().log_file () + "' for writing.");
    else
      ERROR_MSG("Could not locate log file.");
    return -1;
  }

  // Set sigint signal handler
  sigact.sa_handler = connect_sigint;
  sigemptyset (&sigact.sa_mask);
  sigact.sa_flags = 0;
  
  if (sigaction (SIGINT, &sigact, NULL) < 0)
    {
      ERROR_MSG("Signal handler could not be installed.");
      exit (-1);
    }

  try
    {
      // Try to precompile all filters.
      if (precompile_expressions () != 0)
	return -1;
      
      // Start checking for spam mails in the user defined accounts.
      vector<Account> :: iterator cur_account = 
	Preferences :: Instance ().accounts ()->begin ();
      while (cur_account != Preferences :: Instance ().accounts ()->end ())
	{
	  gettimeofday (&tv, &tz);
	  now = tv.tv_sec;
	  
	  // Some versions of ctime () terminate with "\n\0", some don't.
	  string today_ = ctime (&now);
	  if (today_[today_.length () - 1] == '\n')
	    {
	      char* today = (char*)malloc (sizeof (char) * today_.length ());
	      snprintf (today, today_.length (), "%s", ctime (&now));
	      today_ = today;
	      free (today);
	    }
	  logger->print_msg ((string)PACKAGE_VERSION
			    + " querying "
			    + cur_account->usr () + "@"
			    + cur_account->server () + " on "
			    + today_ + ".",
			    3);
	  
	  if (cur_account->check () != 0)
	    {
	      logger->print_err ("Skipping account "
				 + cur_account->usr () + "@"
				 + cur_account->server ()
				 + " due to earlier errors.");
	      return_val = -1;
	    }
	  cur_account++;
	}
    }
  catch (const exception& r_err)
    {
      logger->print_err ("Runtime exception occured: "
			+ (string)r_err.what ());
      return -1;
    }

  // Mailfilter can return the number of pending mails to be
  // downloaded, e.g., when embeddeding it into a script.
  if (Preferences :: Instance ().return_status ())
    return mailbox_status;
  return return_val;
}

// This function is being run before Mailfilter starts to read the
// configuration files and establishes any sort of network connection.
// All initialisation stuff should go in here.

void init_app (void)
{
  mailbox_status = 0;

#ifdef USE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE_NAME, LOCALEDIR);
  textdomain (PACKAGE_NAME);
#endif
}

// The funciton getopts can be used to retrieve user specific command
// line options like '--help', for instance.  The rcfile file path is
// retrieve via get_opts_rcfile as we need to first read the rcfile,
// and then later use this function to override its settings.

void get_opts (int argc, char* argv[])
{
  int option = 0;
  int option_index = 0;

  while ((option = getopt_long (argc, argv, "hL:M:Vv:tir",
				long_options, &option_index)) != -1)
    {
      switch (option)
	{
	case 'h':
	case VALUE_HELP:
	  cout << "Mailfilter filters e-mail and removes spam in one ";
	  cout << "or many POP accounts." << endl;
	  cout << endl;
	  cout << "Usage: " << PACKAGE_NAME << " [OPTION]..." << endl;
	  cout << endl;
	  cout << "If a long option shows an argument as mandatory, ";
	  cout << "then " << endl;
	  cout << "it is mandatory for the equivalent short option ";
	  cout << "also." << endl;
	  cout << endl;
	  cout << "Options:" << endl;
	  cout << "  -h, --help                 ";
	  cout << "Display this help information" << endl;
	  cout << "  -L, --logfile=FILE         ";
	  cout << "Specify logfile location" << endl;
	  cout << "  -M, --mailfilterrc=FILE    ";
	  cout << "Specify rcfile location" << endl;
	  cout << "  -r, --return-value         ";
	  cout << "Enable additional return values" << endl;
	  cout << "  -t, --test                 ";
	  cout << "Simulate deletes" << endl;
	  cout << "  -i, --ignore-time-stamps   ";
	  cout << "Ignore invalid Message-ID time stamps (Do not use unless you know better!)" << endl;
	  cout << "  -v, --verbose=LEVEL        ";
	  cout << "Specify level of verbosity" << endl;
	  cout << "  -V, --version              ";
	  cout << "Display version information" << endl;
	  cout << endl;
	  cout << "Report bugs to ";
	  cout << "<baueran@users.sourceforge.net>." << endl;
	  exit (0);
	  break;
	case 'V':
	case VALUE_VERSION:
          cout << PACKAGE_NAME << " " << PACKAGE_VERSION << endl;
          cout << endl;
	  cout << PACKAGE_COPYRIGHT << endl;
          cout << endl;
          cout << "This is free software; see the source for copying ";
	  cout << "conditions.  There is NO warranty; not even for ";
	  cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR ";
	  cout << "PURPOSE." << endl;
          exit (0);
	  break;
        case 'L':
        case VALUE_LOGFILE:
          Preferences :: Instance ().set_log_file (optarg);
          break;
        case 'v':
        case VALUE_VERBOSE:
	  Preferences :: Instance ().set_verbose_level (atoi (optarg));
          break;
	case 't':
	case VALUE_TEST:
	  Preferences :: Instance ().set_test_mode ("yes");
	  break;
	case 'i':
	case VALUE_TIMESTAMP:
	  Preferences :: Instance().set_ignore_time_stamp();
	  break;
	case 'r':
	case VALUE_RETURN:
	  Preferences :: Instance ().set_return_status (true);
	  break;
	case 'M':
	case VALUE_MAILFILTERRC:
	  Preferences :: Instance ().set_rc_file (optarg);
	  break;
	default:
	  // Command line option not recognised.
	  cerr << "Try '" << argv[0]
	       << " --help' for more information." << endl;
	  exit (-1);
	}
    }
}

// This function precompiles the allow, deny, and score expressions
// of the user's rcfile.  It returns a value different to 0 upon error.

int precompile_expressions (void)
{
  int comp_err = 0;

  try
    {
      vector<Filter> :: iterator cur_filter = 
	Preferences :: Instance ().allow_filters ()->begin ();
      while (cur_filter != 
	     Preferences :: Instance ().allow_filters ()->end ())
	{
	  if ((comp_err = cur_filter->compile ()) != 0)
	    {
	      ERROR_MSG("Could not compile regular expression (allow).");
	      return comp_err;
	    }
	  cur_filter++;
	}
      
      cur_filter = Preferences :: Instance ().deny_filters ()->begin ();
      while (cur_filter != Preferences :: Instance ().deny_filters ()->end ())
	{ 
	  if ((comp_err = cur_filter->compile ()) != 0)
	    {
	      ERROR_MSG("Could not compile regular expression (deny).");
	      return comp_err;
	    }
	  cur_filter++;
	}
      
      vector<Score> :: iterator cur_score = 
	Preferences :: Instance ().score_filters ()->begin ();
      while (cur_score != Preferences :: Instance ().score_filters ()->end ())
	{
	  if ((comp_err = cur_score->compile ()) != 0)
	    {
	      ERROR_MSG("Could not compile regular expression (score).");
	      return comp_err;
	}
	  cur_score++;
	}
    }
  catch (...) {  throw;  }

  return comp_err;
}

// Comapre two strings, but disregard case-sensitivity.  Returns 0, if
// no differences could be determined, a negative integer if s is
// lexicographically before s2, and a positive integer otherwise.
// (See also Stroustrup §20.3.8.)

int cmp_no_case (const string& s, const string& s2)
{
  string :: const_iterator p = s.begin ();
  string :: const_iterator p2 = s2.begin ();
  
  while (p != s.end () && p2 != s2.end ())
    {
      if (toupper (*p) != toupper (*p2))
	return (toupper (*p) < toupper (*p2))? -1 : 1;
      ++p;
      ++p2;
    }

  return (s2.size () == s.size ())? 0 : (s.size () < s2.size ())? -1 : 1;
}

// This function executes the string given in command through a
// POSIX shell.  It returns the first string of the shell's
// output without the trailing '\n'; instead it terminates with
// '\0'.

string exec_shell (const char* command)
{
  FILE *fp;
  const int SIZEBUF = 256;
  static char buf [SIZEBUF];
  string cur_string;
  
  if ((fp = popen (command, "r")) != NULL)
    {
      if (fgets (buf, sizeof (buf), fp) == NULL)
	{
	  ERROR_MSG("popen failed.");
	  exit (-1);
	}
      cur_string = buf;
      
      if (cur_string[cur_string.size () - 1] != '\n')
	{
	  ERROR_MSG("popen failed.");
	  exit (-1);
	}
      else
	// Remove the trailing new line, or it messes up
	// the file paths in the config files.
	cur_string[cur_string.size () - 1] = '\0';
      
      if (pclose(fp) == -1)
	{
	  ERROR_MSG("pclose failed.");
	  exit (-1);
	}
    }
  
  return cur_string;
}

string int_to_string (int val)
{
  string tmp;
  ostringstream ostr;
  ostr << val;
  tmp = ostr.str ();
  return tmp;
}

void connect_sigint (int signo)
{
  sigset_t mask_set;
  sigset_t old_set;
  
  // Mask other signals while we prompt to cerr and finally quit
  signal (SIGINT, connect_sigint);
  sigfillset (&mask_set);
  sigprocmask (SIG_SETMASK, &mask_set, &old_set);

  cout << PACKAGE_NAME \
       << ": Error: Program abort due to signal "
       << signo
       << "." 
       << endl;
  exit (-1);
}
