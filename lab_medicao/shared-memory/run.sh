#!/bin/bash

DATE=$(date '+%Y-%m-%d')

for VERSION in $(seq 1 3); do
  FOLDER=$(echo $DATE.$VERSION)
  gcc shm.c -o shm
  ./shm
  mkdir ./output/$FOLDER

  mv output/array-read ./output/$FOLDER/
  mv output/array-write ./output/$FOLDER/
  mv output/shm-read ./output/$FOLDER/
  mv output/shm-write ./output/$FOLDER/

  cd ./output/$FOLDER/
    echo array-read,shm-read,array-write,shm-write >> out.csv
    paste -d',' array-read shm-read array-write shm-write >> out.csv
  cd - >> /dev/null
done
