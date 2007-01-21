// preferences.hh - source file for the mailfilter program
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

#ifndef PREFERENCES_HH
#define PREFERENCES_HH

#include <string>
#include <vector>
#include <fstream>
#include "defines.hh"
#include "socket.hh"
#include "filter.hh"
#include "score.hh"
#include "account.hh"

using namespace std;

class Preferences
{
protected:
  ifstream         prefs_stream;
  vector<Filter>   allows;
  vector<Filter>   denies;
  vector<Score>    scores;
  vector<Account>  accnts;
  Account          cur_account;
  string           prefs_file_name;
  string           log_file_name;
  string           headers_file_name;
  int              icase;
  bool             norm;
  bool             test;
  bool             show_headers;
  bool             del_duplicates;
  bool             ret_status;
  int              high_score;
  unsigned         time_out_val;
  int              max_size;
  Size_score       size_score;
  int              max_size_friends;
  int              max_line_length;
  int              rreg_type;
  int              verbosity;
  int              conn_type;
  int              negative_allows;
  int              negative_denies;
  int              negative_scores;
  // These flags indicate whether a value was already set, or
  // whether it's still set to the default values defined by the
  // constructor.
  bool             verbosity_changed;
  bool             test_changed;
public:
  Preferences                        ();
  static Preferences&
                   Instance          ();
  void             init              (void);
  void             kill              (void);
  bool             open              (const char*);
  bool             load              (void);
  void             add_deny_rule     (const char*,
				      const char*,
				      const char*);
  void             add_allow_rule    (const char*,
				      const char*,
				      const char*);
  void             add_score         (const char*,
				      int,
				      const char*,
				      const char*);
  int              neg_allows        (void);
  int              neg_denies        (void);
  void             set_rc_file       (const char*);
  string           rc_file           (void);
  void             set_log_file      (const char*);
  string           log_file          (void);
  void             set_verbose_level (int);
  int              verbose_level     (void);
  void             set_headers_file  (const char*);
  string           headers_file      (void);
  void             set_default_case  (const char*);
  int              default_case      (void);
  void             set_reg_type      (const char*);
  int              reg_type          (void);
  void             set_server        (const char*);
  void             set_usr           (const char*);
  void             set_passwd        (const char*);
  void             set_protocol      (const char*);
  void             set_connection    (unsigned int = POSIX_SOCKETS) 
                                     __attribute__ ((unused));
  void             set_port          (unsigned int);
  unsigned int     time_out          (void);
  void             set_time_out      (unsigned int);
  bool             delete_duplicates (void);
  void             set_del_duplicates(const char*);
  int              max_size_allow    (void);
  void             set_max_size_allow(int);
  int              max_size_deny     (void);
  void             set_max_size_deny (int);
  Size_score       max_size_score    (void);
  void             set_max_size_score(int, int);
  int              highscore         (void);
  void             set_highscore     (int);
  bool             normal            (void);
  void             set_normal        (const char*);
  bool             test_mode         (void);
  void             set_test_mode     (const char*);
  int              maxlength         (void);
  void             set_maxlength     (int);
  bool             return_status     (void);
  void             set_return_status (bool);
  vector<Account>* accounts          (void);
  vector<Filter>*  allow_filters     (void);
  vector<Filter>*  deny_filters      (void);
  vector<Score>*   score_filters     (void);
};

#endif
