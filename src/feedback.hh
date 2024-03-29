// feedback.hh - source file for the mailfilter program
// Copyright (c) 2000 - 2022  Andreas Bauer <baueran@gmail.com>
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

#ifndef FEEDBACK_HH
#define FEEDBACK_HH

#include <fstream>
#include <string>

using namespace std;

class Feedback
{
private:
  ofstream         log_file;
  ofstream         header_file;
  static Feedback* _instance;

  string           timestamp         (void);
  
public:
  static Feedback* Instance          (void);
                   ~Feedback         (void);
  bool             open              (const char*);
  bool             print_msg         (const string, int);
  bool             print_err         (const string, int = 1);
  bool             print_header      (const string);
};

#endif
