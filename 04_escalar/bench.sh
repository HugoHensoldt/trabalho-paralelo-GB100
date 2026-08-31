#!/bin/bash
N=${1:-20000000}
echo "threads,versao,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  for P in critical atomic local reduction; do
    TEMPO=$(./exemplo_escalar_$P $N | grep 'tempo =' | grep -oE '[0-9]+\.[0-9]+')
    echo "$T,$P,$TEMPO"
  done
done
