#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Needs getstats.pl
# Checks the filters and how often they were active!
# Then sorts the ouput in ascending order.
# E.g. 
#  32 Applied  filter: '^Received:.*\.tw[[:space:]]']
#  35 Applied  filter: '^Received:.*\.br']
#  81 Applied  filter: 'Content-Type:.*text/html.*']
# Used also by prozente.pl
#####
zgrep Deleted log/mailfilter.log* | getstats.pl | sort | uniq -c | sort -n
