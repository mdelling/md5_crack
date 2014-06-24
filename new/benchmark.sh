#!/bin/bash

# Build objects
make

# Build benchmark tool
gcc -std=gnu99 -Os benchmark_block_threaded.c *.o -o benchmark_block_threaded

# Time runs
clear
time ./benchmark_block_threaded 100 1 && echo
time ./benchmark_block_threaded 1000 1 && echo
time ./benchmark_block_threaded 100000 1 && echo
time ./benchmark_block_threaded 100 && echo
time ./benchmark_block_threaded 1000 && echo
time ./benchmark_block_threaded 100000 && echo

# Cleanup
rm benchmark_block_threaded
