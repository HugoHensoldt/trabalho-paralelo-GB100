#!/bin/bash
# uso: ./bench_sweep.sh
# Curva de desempenho variando n (28..36), alem de F(25): aqui os tempos
# de uma unica chamada ja ficam na casa de dezenas de ms a segundos, entao
# nao precisa de repeticao interna (REPETICOES=1) -- 3 execucoes externas
# do processo, mediana, bastam para uma leitura estavel.
echo "threads,n,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  for N in 28 30 32 34 36; do
    T1=$(./fib_task $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    T2=$(./fib_task $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    T3=$(./fib_task $N | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
    MEDIANA=$(printf '%s\n%s\n%s\n' "$T1" "$T2" "$T3" | sort -n | sed -n '2p')
    echo "$T,$N,$MEDIANA"
  done
done
