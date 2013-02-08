#!/bin/bash

if [ "$1x" != "x" ];
then
  while [ 1 ]
  do
    ./ddoslog.bin $1 $2 $3
  done
else
./ddoslog.bin
fi
