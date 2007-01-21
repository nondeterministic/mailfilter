#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Checks for deleted mails and send the filter applied
# Good for crontab to see if mails you should
# get were deleted
#####
grep Deleted /home/kay/log/mailfilter.log | mail -s "Mailfilter" kay

