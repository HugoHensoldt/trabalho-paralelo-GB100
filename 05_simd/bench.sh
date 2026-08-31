#!/bin/bash
# uso: ./bench.sh <num_threads>
export OMP_NUM_THREADS=${1:-4}
echo "n,versao,tempo_s"
for N in 100000 1000000 5000000 10000000 50000000 100000000; do
  for P in on off; do
    TEMPO=$(./exemplo_simd_$P $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    echo "$N,$P,$TEMPO"
  done
done
