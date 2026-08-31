#!/bin/bash

gcc -fopenmp -O2 exemplo_escalar_critical.c   -o exemplo_escalar_critical
gcc -fopenmp -O2 exemplo_escalar_atomic.c     -o exemplo_escalar_atomic
gcc -fopenmp -O2 exemplo_escalar_local.c      -o exemplo_escalar_local
gcc -fopenmp -O2 exemplo_escalar_reduction.c  -o exemplo_escalar_reduction

echo '*** COMPILACAO COMPLETADA ***'
