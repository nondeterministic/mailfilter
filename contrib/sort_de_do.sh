#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Sorts a file which only contains filters for
# domains, such as
# DENY=^(From|To|Cc):.*@.*handmark\.net
# DENY=^(From|To|Cc):.*@.*homebets\.com
# DENY=^(From|To|Cc):.*@.*homestar\.us
# by the top level domain, then the first level domain etc.
#####
cat deny_domain | sort -bdf -t . +3 +2.1 -3 | more
