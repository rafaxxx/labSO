#!/bin/bash

N_SAMPLE=1000
DATE=$(date '+%Y-%m-%d')
FOLDER=$(echo $DATE.$VERSION)
rm -rf ./output/$FOLDER
mkdir -p ./output/$FOLDER
OUTPUT="./output/$FOLDER"
M_SIZES="1 128 256 512"
shopt -s extglob

run_shm() {
  gcc shared-memory/shm.c -o shared-memory/shm
  ./shared-memory/shm $1 $2

  cp shared-memory/output/array-read  $OUTPUT/array-read-$1
  cp shared-memory/output/shm-read    $OUTPUT/shm-read-$1
  cp shared-memory/output/array-write $OUTPUT/array-write-$1
  cp shared-memory/output/shm-write   $OUTPUT/shm-write-$1
}

run_signal(){
  SIGNAL_TIME_MERGES=signal/output/signal-time-merges
  SIGNAL_TIME_RESULT=signal/output/signal-time-result
  SIGNAL_CLIENT=signal/output/signal-client
  SIGNAL_SERVER=signal/output/signal-server

  #cleanup
  cat /dev/null > $SIGNAL_TIME_MERGES
  cat /dev/null > $SIGNAL_TIME_RESULT
  cat /dev/null > $SIGNAL_CLIENT
  cat /dev/null > $SIGNAL_SERVER

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
  mv $SIGNAL_TIME_RESULT $OUTPUT/signal-$1
}

shm(){
  for M_SIZE in $(echo $M_SIZES); do
    run_shm $M_SIZE $N_SAMPLE
  done
}

signal(){
  for M_SIZE in $(echo $M_SIZES); do
    run_signal $M_SIZE $N_SAMPLE
  done
}

shm &
signal &
wait

cd $OUTPUT
  for M_SIZE in $(echo $M_SIZES); do
    echo array-read,shm-read,array-write,shm-write,signal >> out-$M_SIZE.csv
    paste -d',' array-read-$M_SIZE \
                shm-read-$M_SIZE \
                array-write-$M_SIZE \
                shm-write-$M_SIZE \
                signal-$M_SIZE \
    >> out-$M_SIZE.csv
  done
cd - >> /dev/null
