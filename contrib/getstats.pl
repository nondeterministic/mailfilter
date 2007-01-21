#!/usr/bin/perl -w
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Searches for the String Applied in the logfiles.
# Needs SHOW_HEADERS set to yes
#####
use strict;
while (<>) 
{
        if (/Applied(.*)$/) 
        {
                print "Applied $1\n";
        }
}

