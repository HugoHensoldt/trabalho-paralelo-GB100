#!/bin/bash
# uso: ./run.sh <tamanho_do_vetor>
NUM=${1:-10000000}

echo "===== com #pragma omp simd (N=$NUM) ====="
./exemplo_simd_on $NUM

echo "===== sem #pragma omp simd (N=$NUM) ====="
./exemplo_simd_off $NUM
