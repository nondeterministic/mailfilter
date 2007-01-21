#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Calculates the amount of emails examined by mailfilter
# This only makes sense if you examine mails only once.
# If you do not download and then delete the emails on the
# server, this script will give you the wrong number.
#####
zgrep Examining log/mailfilter.log* | awk '{x=x+$3}; END {print x}' 
