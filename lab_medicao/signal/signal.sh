#!/bin/bash

SIGNAL_TIME_MERGES=signal/output/signal-time-merges
SIGNAL_TIME_RESULT=signal/output/signal-time-result
SIGNAL_CLIENT=signal/output/signal-client
SIGNAL_SERVER=signal/output/signal-server
cat /dev/null > $SIGNAL_TIME_MERGES
cat /dev/null > $SIGNAL_TIME_RESULT

gcc signal/signal-get-times.c -o signal/signal-get-times
./signal/signal-get-times $1 $2

paste -d',' $SIGNAL_CLIENT $SIGNAL_SERVER >> $SIGNAL_TIME_MERGES

while read -r line; do

  BEGIN_SEC=$(echo $line | cut -d',' -f1 | cut -d':' -f1)
  BEGIN_NSEC=$(echo $line | cut -d',' -f1 | cut -d':' -f2)
  END_SEC=$(echo $line | cut -d',' -f2 | cut -d':' -f1)
  END_NSEC=$(echo $line | cut -d',' -f2 | cut -d':' -f2)

  DIFF_SEC=$(( $END_SEC - $BEGIN_SEC ))
  DIFF_NSEC=$(( $END_NSEC - $BEGIN_NSEC ))

  ABS_DIFF_SEC=${DIFF_SEC#-}
  ABS_DIFF_NSEC=${DIFF_NSEC#-}

  echo "$ABS_DIFF_SEC.$ABS_DIFF_NSEC" >> $SIGNAL_TIME_RESULT
done < "$SIGNAL_TIME_MERGES"
