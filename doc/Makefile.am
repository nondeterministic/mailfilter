# Makefile.am: help build documentation files
# $Id: Makefile.am,v 1.11.2.1.2.2 2004/01/25 12:56:37 baueran Exp $
#
# Copyright (c) 2000 - 2004  Andreas Bauer <baueran@in.tum.de>
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


EXTRA_DIST = FAQ supported_servers

@MAINT@doxygen: Doxyfile
@MAINT@	doxygen Doxyfile

dist-hook:
	if test -d $(srcdir)/api; then \
		mkdir $(distdir)/api ;\
	fi
	if test -d $(srcdir)/api/html; then \
		mkdir $(distdir)/api/html ;\
		cp $(srcdir)/api/html/* $(distdir)/api/html ;\
	fi
	if test -d $(srcdir)/api/latex; then \
		mkdir $(distdir)/api/latex ;\
		cp $(srcdir)/api/latex/* $(distdir)/api/latex ;\
	fi
	if test -d $(srcdir)/api/man; then \
		mkdir $(distdir)/api/man ;\
		cp -r $(srcdir)/api/man/* $(distdir)/api/man ;\
	fi
	if test -d $(srcdir)/api/rtf; then \
		mkdir $(distdir)/api/rtf ;\
		cp $(srcdir)/api/rtf/* $(distdir)/api/rtf ;\
	fi
