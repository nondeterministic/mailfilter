# Makefile.am: help build program sources
# $Id: Makefile.am,v 1.9.2.6.2.22 2006/12/31 21:44:18 baueran Exp $

# Copyright (c) 2000 - 2020  Andreas Bauer <baueran@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
# USA.

EXTRA_DIST = rcfile.yy rfc822.yy

AM_CXXFLAGS = -Wall
YFLAGS = -d -v
AM_LFLAGS = -+ -i

bin_PROGRAMS = mailfilter

# Some dependencies to invoke flex + bison before compilation of lex output starts:
rcfile.cc: rcfile.ll rcparser.hh
	   $(LEX) $(AM_LFLAGS) --prefix=rc -o$@ $<

rcparser.hh: rcparser.cc
rcparser.cc: rcfile.yy
		   $(YACC) $(YFLAGS) --file-prefix=rc -Dapi.prefix={rc} --defines=rcparser.hh $<;  \
		   mv rc.tab.c rcparser.cc

# Almost the same as above, but this time for the RFC 822 parser:
rfc822.cc: rfc822.ll rfc822parser.hh
	   $(LEX) $(AM_LFLAGS) --prefix=rfc -o$@ $<

rfc822parser.hh: rfc822parser.cc
rfc822parser.cc: rfc822.yy
		   $(YACC) $(YFLAGS) --file-prefix=rfc -Dapi.prefix={rfc} --defines=rfc822parser.hh $<;  \
		   mv rfc.tab.c rfc822parser.cc

CLEANFILES = *.output Makefile.in rcfile.cc rcfile.ll.bak rcparser.cc rcparser.hh rfc822.cc rfc822parser.cc rfc822parser.hh

# If this gets updated, remember to update the doxygen.in config file!
mailfilter_SOURCES =	md5c.c md5.h                              \
			defines.hh                                \
			rcfile.ll rcfile.hh                       \
			rcparser.hh rcparser.cc                   \
			rfc822.ll rfc822parser.hh rfc822parser.cc \
			mailfilter.hh mailfilter.cc               \
			header.hh header.cc                       \
			weeder.hh weeder.cc                       \
			preferences.hh preferences.cc             \
			feedback.hh feedback.cc                   \
			filter.hh filter.cc                       \
			score.hh score.cc                         \
			account.hh account.cc                     \
			protocol.hh protocol.cc                   \
			connection.hh                             \
			socket.hh socket.cc                       \
			pop3.hh pop3.cc                           \
			apop.hh apop.cc                           \
			imap.hh imap.cc

if !GETOPT
mailfilter_SOURCES += getopt.c getopt1.c getopt.h
endif

AM_CPPFLAGS = -I$(includedir)                                     \
	      -I$(srcdir) -I$(top_srcdir)/include -I$(top_srcdir) \
	      -DLOCALEDIR=\"$(datadir)/locale\"                   \
	      -I$(top_srcdir)/intl                                \
	      -I$(top_builddir) -I$(top_builddir)/include -I.

LIBS = @LEXLIB@ @LIBS@
