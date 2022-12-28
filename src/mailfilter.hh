// mailfilter.hh - source file for the mailfilter program
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

#ifndef MAILFILTER_HH
#define MAILFILTER_HH

#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;

// Mailfilter's command line options and arguments.

#define VALUE_HELP                      1
#define VALUE_VERBOSE                   2
#define VALUE_MAILFILTERRC              3
#define VALUE_LOGFILE                   4
#define VALUE_VERSION                   5
#define VALUE_TEST                      6
#define VALUE_RETURN                    7
#define VALUE_TIMESTAMP                 8
#define VALUE_SKIP_SSL_VERIFY           9
#define VALUE_MAX_MESSAGES             10

#define ERROR_MSG(msg)  \
  cerr << PACKAGE_NAME  \
       << ": Error: "   \
       << msg           \
       << endl

#endif
