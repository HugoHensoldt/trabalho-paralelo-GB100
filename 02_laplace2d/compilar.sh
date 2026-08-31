#!/bin/bash
set -e

cd "$(dirname "$0")"

# Experimento 1 (malha 4096x4096, sem calculo de erro): iter_max=50 e iter_max=500.
# laplace2d_gcc_i500 tambem serve de baseline "loops separados" do experimento 3.
gcc -fopenmp -O2 -DITER_MAX=50  -o laplace2d_gcc_i50  laplace2d_gcc.c
gcc -fopenmp -O2 -DITER_MAX=500 -o laplace2d_gcc_i500 laplace2d_gcc.c

# Experimento 2 (malha 1024x1024, sem calculo de erro): iter_max=50 e iter_max=500.
gcc -fopenmp -O2 -DITER_MAX=50  -o laplace2d_1k_gcc_i50  laplace2d_1k_gcc.c
gcc -fopenmp -O2 -DITER_MAX=500 -o laplace2d_1k_gcc_i500 laplace2d_1k_gcc.c

# Experimento 3 (fusao das duas regioes paralelas em uma so): iter_max=500.
gcc -fopenmp -O2 -DITER_MAX=500 -o laplace2d_1loop_gcc_i500 laplace2d_1loop_gcc.c

echo '*** COMPILACAO OK ***'
