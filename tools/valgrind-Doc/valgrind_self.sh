#!/bin/bash


exec=$1
ourputlogfile=valgrind_log.txt

valgrind --tool=memcheck --log-file=$ourputlogfile -s --leak-check=yes  --show-reachable=yes $exec
cat $ourputlogfile



# tmp/mnt/usb1_1/install/valgrind --tool=memcheck --log-file=valgrind_log.txt -s --leak-check=yes  --show-reachable=yes /var/apps/a.out

