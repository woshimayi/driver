#!/bin/bash

exec=$1
callpath=/usr/local/bin/ms_print

valgrind --tool=massif $exec

$callpath massif.out.*