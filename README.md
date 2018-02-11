# Mailfilter [![Build Status](https://travis-ci.org/nondeterministic/mailfilter.png?branch=master)](https://travis-ci.org/nondeterministic/mailfilter)

Mailfilter is a flexible utility for Unix-like operating systems to
get rid of unwanted spam mails, before having to go through the
trouble of downloading them into the local computer.  It offers
support for one or many POP accounts and is especially useful for
dialup connections via modem, ISDN, etc.

Mailfilter connects to any POP mail box and compares part of its
content to a set of user defined filter rules.  That way the spam gets
deleted directly on the mail server.

With Mailfilter you can define your own filters (rules) to determine
which e-mails should be delivered and which are considered waste.
Rules are regular expressions, so you can make use of familiar options
from other mail delivery programs such as e.g. procmail.

Mailfilter is released under the GPL with the additional exemption            
that compiling, linking, and/or using OpenSSL is allowed.  For more
information, see the COPYING file provided with the Mailfilter program.

The latest releases of Mailfilter can be obtained from this web page:

       http://mailfilter.sourceforge.net/

For installation instructions please refer to the INSTALL document and
read the platform specific information and the FAQ in the doc/
directory.  Further information can be found in the mailfilter(1) man
page.

Enjoy Mailfilter!
