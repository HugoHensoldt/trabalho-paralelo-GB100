#!/bin/bash
#
# Benchmark: multiplicacao de matrizes N=4096 comparando as clausulas de
# escalonamento static/dynamic/guided com chunk=2 e chunk=100.
#
# OMP_NUM_THREADS e fixado no numero de nucleos FISICOS da maquina de teste
# (4 no Intel i5-1135G7 usado neste laboratorio -- ver README.md do
# Laboratorio_2). O programa usa `schedule(runtime)`, entao o par
# (modo, chunk) e definido aqui via OMP_SCHEDULE, sem recompilar.
#
# Uso:
#   ./run_benchmark.sh [num_threads] [repeticoes]
#
set -e

cd "$(dirname "$0")"

NUM_THREADS="${1:-4}"
REPS="${2:-3}"
EXEC=./mult_matriz
OUT=resultados.csv

if [ ! -x "$EXEC" ]; then
    ./compilar.sh
fi

export OMP_NUM_THREADS="$NUM_THREADS"

echo "schedule,chunk,rep,tempo_s" > "$OUT"

for sched in static dynamic guided; do
    for chunk in 2 100; do
        export OMP_SCHEDULE="${sched},${chunk}"
        for rep in $(seq 1 "$REPS"); do
            linha=$("$EXEC")
            tempo=$(echo "$linha" | awk '{for(i=1;i<=NF;i++) if ($i ~ /^tempo_s=/) print substr($i, 9)}')
            echo "${sched},${chunk},${rep},${tempo}" | tee -a "$OUT"
        done
    done
done

echo
echo "=== Media por configuracao (OMP_NUM_THREADS=${NUM_THREADS}, REPS=${REPS}) ==="
awk -F, 'NR>1 {sum[$1","$2]+=$4; cnt[$1","$2]++}
         END {print "schedule,chunk,tempo_medio_s";
              for (k in sum) printf "%s,%.4f\n", k, sum[k]/cnt[k]}' "$OUT" | sort -t, -k1,1 -k2,2n
