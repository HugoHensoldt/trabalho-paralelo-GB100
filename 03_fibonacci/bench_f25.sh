#!/bin/bash
# uso: ./bench.sh
# Curva de desempenho para F(25): varia OMP_NUM_THREADS, com o programa
# repetindo internamente o calculo (REPETICOES) para uma leitura de tempo
# estavel, ja que uma unica chamada de fib(25) dura poucos milissegundos.
# 3 repeticoes externas do processo, mediana.
N=25
REPETICOES=30
echo "threads,n,tempo_s"
for T in 1 2 4 8; do
  export OMP_NUM_THREADS=$T
  T1=$(./fib_task $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  T2=$(./fib_task $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  T3=$(./fib_task $N $REPETICOES | grep -oE 'tempo = [0-9]+\.[0-9]+' | grep -oE '[0-9]+\.[0-9]+')
  MEDIANA=$(printf '%s\n%s\n%s\n' "$T1" "$T2" "$T3" | sort -n | sed -n '2p')
  echo "$T,$N,$MEDIANA"
done
