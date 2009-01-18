# Makefile.am: help build program sources
# $Id: Makefile.am,v 1.9.2.6.2.22 2006/12/31 21:44:18 baueran Exp $

# Copyright (c) 2000 - 2009  Andreas Bauer <baueran@gmail.com>
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

# Some dependencies to invoke flex + bison before compilation of
# lex output starts:
rcfile.cc: rcfile.ll rcparser.hh
	   $(LEX) $(AM_LFLAGS) -Prc -o$@ $<

# The final `touch' is necessary to be able to invoke flex/bison more
# than once and to not confuse the ../ylwrap script.
rcparser.hh: y.tab.c
y.tab.c: rcfile.yy
	   $(YACC) -p rc $(YFLAGS) -o$@ $<;   \
	   mv -f y.tab.c rcparser.cc;         \
	   mv -f y.tab.h rcparser.hh;         \
	   $(CXXCOMPILE) -c rcparser.cc;      \
	   touch y.tab.c

# Almost the same as above, but this time for the RFC 822 parser:
rfc822.cc: rfc822.ll rfc822parser.hh
	   $(LEX) $(AM_LFLAGS) -Prfc -o$@ $<

rfc822parser.hh: rfc822parser.cc
rfc822parser.cc: rfc822.yy
		   $(YACC) $(YFLAGS) -p rfc -o$@ $<;  \
		   mv -f y.tab.h rfc822parser.hh;     \
		   $(CXXCOMPILE) -c rfc822parser.cc;  \
		   touch y.tab.c

# This thing is a workaround to avoid compile errors.
# We always re-generate the source from the flex/bison input, so it
# always matches the installed versions and does not lead to errors.
CLEANFILES = rcfile.cc rcparser.hh rcparser.cc y.tab.c ylwrap    \
             rfc822parser.output rfc822parser.cc rfc822parser.hh \
	     rfc822.cc y.output

nodist_mailfilter_SOURCES = rcfile.cc rcparser.hh y.tab.c rfc822.cc

nodist_mailfilter_OBJECTS = y.tab.$(OBJEXT)

# Looks like automake still wants to distribute rcfile.cc, even if it
# is in nodist_*_sources.
dist-hook:
	rm -f $(distdir)/rcfile.cc                              \
	      $(distdir)/rfc822parser.cc                        \
	      $(distdir)/rfcparser.cc

# If this gets updated, remember to update the doxygen.in config file!
mailfilter_SOURCES =	md5c.c md5.h                            \
			defines.hh                              \
			rcfile.ll rcfile.hh                     \
			rfc822.ll                               \
			mailfilter.hh mailfilter.cc             \
			header.hh header.cc                     \
			weeder.hh weeder.cc                     \
			preferences.hh preferences.cc           \
			feedback.hh feedback.cc                 \
			filter.hh filter.cc                     \
			score.hh score.cc                       \
			account.hh account.cc                   \
			protocol.hh protocol.cc                 \
			connection.hh                           \
			socket.hh socket.cc                     \
			pop3.hh pop3.cc                         \
			apop.hh apop.cc                         \
			imap.hh imap.cc

if !GETOPT
mailfilter_SOURCES += getopt.c getopt1.c getopt.h
endif

mailfilter_LDADD = rcparser.o rfc822parser.o

INCLUDES = -I$(includedir)                                      \
	-I$(srcdir) -I$(top_srcdir)/include -I$(top_srcdir)     \
	-DLOCALEDIR=\"$(datadir)/locale\"                       \
	-I$(top_srcdir)/intl                                    \
	-I$(top_builddir) -I$(top_builddir)/include -I.

LIBS = @LEXLIB@ @LIBS@
