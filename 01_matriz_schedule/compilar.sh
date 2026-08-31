#!/bin/bash
set -e

gcc -fopenmp -O2 -o mult_matriz mult_matriz.c

echo '*** COMPILACAO OK ***'
