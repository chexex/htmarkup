#!/usr/bin/env bash

print_ok() {
    echo OK
}

print_skipped() {
    echo SKIPPED
}

print_failed() {
    if [[ -t 1 ]]; then
        echo -e '\033[1mFAILED\033[0m'
    else
        echo FAILED
    fi
}

rm -rf logs
mkdir logs

NTESTS=$( ls T-[0-9][0-9]*.sh | wc -l | awk '{ print $1 }' )
i=0
NFAILED=0

if [[ $NTESTS -eq 0 ]]; then
    echo "No tests found!" >&2
    exit 0
fi

for t in $( ls T-[0-9][0-9]*.sh | sort ) ; do
  printf "[%2d/%-2d] %-30s ... " $(( ++i )) $NTESTS $t
  if echo $t | grep -q _DISABLED; then
      print_skipped
  else
      BATCHMODE=1 ./$t &>logs/$t.log && print_ok || { print_failed; (( NFAILED++ )); }
  fi
done

[[ $NFAILED -eq 0 ]] || exit 1

echo SUCCESS

