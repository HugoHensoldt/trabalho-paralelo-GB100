#!/bin/bash
# uso: ./bench_sweep_cutoff.sh
# Mesma varredura de bench_sweep.sh, mas para a versao com corte de
# granularidade, incluindo n maiores (40, 44) que seriam inviaveis sem corte.
echo "threads,n,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  for N in 28 30 32 34 36 40 44; do
    T1=$(./fib_task_cutoff $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    T2=$(./fib_task_cutoff $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    T3=$(./fib_task_cutoff $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    MEDIANA=$(printf '%s\n%s\n%s\n' "$T1" "$T2" "$T3" | sort -n | sed -n '2p')
    echo "$T,$N,$MEDIANA"
  done
done
