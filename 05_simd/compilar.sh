#!/bin/bash
# -fno-tree-vectorize -fno-tree-slp-vectorize: a partir do GCC 12 o -O2 ja
# vetoriza lacos simples sozinho. Essas flags desligam essa vetorizacao
# automatica para que a UNICA fonte de vetorizacao seja o "#pragma omp simd"
# presente em exemplo_simd_on.c (ausente em exemplo_simd_off.c).

FLAGS="-fopenmp -O2 -fno-tree-vectorize -fno-tree-slp-vectorize"

gcc $FLAGS exemplo_simd_on.c   -o exemplo_simd_on
gcc $FLAGS exemplo_simd_off.c  -o exemplo_simd_off

echo '*** COMPILACAO COMPLETADA ***'
