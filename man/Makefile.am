# Makefile.am: help build documentation files
# $Id: Makefile.am,v 1.4.2.1.2.4 2007/01/01 15:32:29 baueran Exp $
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

man_MANS = mailfilter.1 mailfilterrc.5 mailfilterex.5

mailfilter_manpages.ps:
	for i in $(man_MANS); \
	do \
	groff -t -e -mandoc -Tps $$i > $$i.ps; \
	groff -t -e -mandoc -Tascii $$i > $$i.txt; \
	groff -t -e -mandoc -Thtml $$i > $$i.html; \
	done;

pdf-am: mailfilter.1.ps
	for i in *.ps; do ps2pdf $$i; done;

clean:
	rm -f *.pdf *.html *.ps *.txt

EXTRA_DIST = $(man_MANS)
