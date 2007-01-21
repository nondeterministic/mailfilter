#!/usr/bin/perl -w
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Get's the mail client from the logfile
# by searching for /X-Mailer in the mailheader
# Needs SHOW_HEADERS set to yes
#####
use strict;
while (<>) 
{
        if (/X-Mailer(.*)$/) 
        {
                print "X-Mailer $1\n";
        }
}

