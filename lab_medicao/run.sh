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
  gcc signal/signal.c -o signal/signal
  ./signal/signal $1 $2
  mv signal/output/signal $OUTPUT/signal-$1
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
$!
shm &
pid1=$!
signal &
pid2=$!

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
