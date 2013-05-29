#!/usr/bin/env bash

#
# Task: SRCH-432
# It's usefull to make qclassify silent: sometimes it starts from cron
# and all of it's output going to mailbox.
#

. common.incl.sh

$BINDIR/idx_phrases -c ../T-05-silent.config.xml &>log

[ -e log ] || err "WTF?! logfile must be created"
if [ -s log ]; then
  err "log must be empty since 'silent'-mode"
fi
