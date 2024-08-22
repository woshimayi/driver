#!/bin/bash

# gcc -g -O0 pthread_test-1.c 


exec=$1
output=log.txt
outputcallgrind=

rm callgrind.out.*

#valgrind --tool=memcheck --log-file=$output -s --leak-check=yes $exec 

#valgrind  --log-file=$output -s --leak-check=yes $exec 

# valgrind  --log-file=$output    --leak-check=yes $exec 

valgrind  --tool=callgrind  $exec 

callgrind_annotate  --inclusive=yes --tree=both --auto=yes  callgrind.out.*  > $output


python3.12 ./../wget/gprof2dot/gprof2dot.py -f callgrind -n 10 callgrind.out.*  > valgrind_call.dot


dot -Tpng valgrind_call.dot -o valgrind.png