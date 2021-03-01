#!/bin/bash

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 18 19 20
do
   dest="/tmp/"
   dest+=$i
   dest+=$1
   ./client $2 $3 $1 $dest &
done
