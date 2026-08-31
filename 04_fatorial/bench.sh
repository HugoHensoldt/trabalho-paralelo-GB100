#!/bin/bash
echo "threads,versao,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  for P in critical local reduction; do
    TEMPO=$(./fatorial_$P | grep 'Tempo decorrido' | grep -oE '[0-9]+\.[0-9]+')
    echo "$T,$P,$TEMPO"
  done
done
