#!/bin/sh
#####
# (C) Kay Schulz
# Licensed under the same terms as Mailfilter, GPL v2 or later.
# Use at your own risk!
# Needs getmailer.pl
# Calculates the amount how often a mailclient
# was used.
# E.g.
#  28 The Bat! (v1.33)
#  29 Internet Mail Service
#  29 The Bat! (v1.53d)
#  33 Wmx-0.3
#  36 Microsoft Outlook CWS,
# Is not perfect, because the  naming of the mailclients,
# particularly for the different OE versions is not unique
# But it gives an overview.
#####
zgrep -i X-Mailer log/mailfilter.log* | getmailer.pl | awk '{print $3 " " $4 " " $5}' | sort | uniq -c | sort -n
