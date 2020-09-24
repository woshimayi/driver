#!/bin/bash

start=$(date +%s)
# nmap man.linuxde.net &> /dev/null
echo "111111111"

if [ 1 == 0 ]; then
	exit 8
fi

end=$(date +%s)
difference=$(( end - start ))
echo $difference seconds.
