#!/bin/bash

gcc -fopenmp -O2 fatorial_critical.c   -o fatorial_critical
gcc -fopenmp -O2 fatorial_local.c      -o fatorial_local
gcc -fopenmp -O2 fatorial_reduction.c  -o fatorial_reduction

echo '*** COMPILACAO COMPLETADA ***'
