#!/bin/bash
# uso: ./run.sh <tamanho_do_vetor>
NUM=${1:-10000000}

echo "===== critical (N=$NUM) ====="
./exemplo_escalar_critical $NUM

echo "===== atomic (N=$NUM) ====="
./exemplo_escalar_atomic $NUM

echo "===== acumulador local (N=$NUM) ====="
./exemplo_escalar_local $NUM

echo "===== reduction (N=$NUM) ====="
./exemplo_escalar_reduction $NUM
