#!/bin/bash

gcc -fopenmp -O2 fib_task.c        -o fib_task
gcc -fopenmp -O2 fib_task_cutoff.c -o fib_task_cutoff

echo '*** COMPILACAO COMPLETADA ***'
