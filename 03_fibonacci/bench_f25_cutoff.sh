#!/bin/bash
# uso: ./bench_cutoff.sh
# Mesma curva (F(25) x threads), para a versao com corte de granularidade.
# Como cada chamada individual dura microssegundos nessa versao, o numero
# de repeticoes internas eh bem maior que no bench.sh do baseline.
N=25
REPETICOES=2000
echo "threads,n,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  T1=$(./fib_task_cutoff $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  T2=$(./fib_task_cutoff $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  T3=$(./fib_task_cutoff $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  MEDIANA=$(printf '%s\n%s\n%s\n' "$T1" "$T2" "$T3" | sort -n | sed -n '2p')
  echo "$T,$N,$MEDIANA"
done
