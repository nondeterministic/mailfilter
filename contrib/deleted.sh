#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Needs runit.sh
# Calculates the amount of deleted mails by mailfilter
#####
runit.sh | awk '{x=x+$1}; END{print x}'
