#!/bin/bash
#
# Curvas de desempenho da Tarefa 2 (Laplace 2D), de 1 ate o numero de
# nucleos FISICOS da maquina de teste (4 no Intel i5-1135G7 -- ver
# README.md do Laboratorio_2):
#
#   laplace2d_gcc_i50       -- exp. 1, malha 4096x4096, iter_max=50
#   laplace2d_gcc_i500      -- exp. 1, malha 4096x4096, iter_max=500
#                               (tambem baseline "loops separados" do exp. 3)
#   laplace2d_1k_gcc_i50    -- exp. 2, malha 1024x1024, iter_max=50
#   laplace2d_1k_gcc_i500   -- exp. 2, malha 1024x1024, iter_max=500
#   laplace2d_1loop_gcc_i500-- exp. 3, loops fundidos, iter_max=500
#
# Uso:
#   ./bench.sh [max_threads] [repeticoes]
#
set -e

cd "$(dirname "$0")"

MAX_THREADS="${1:-4}"
REPS="${2:-5}"
OUT="${3:-resultados.csv}"

PROGRAMS="${PROG_LIST:-laplace2d_gcc_i50 laplace2d_gcc_i500 laplace2d_1k_gcc_i50 laplace2d_1k_gcc_i500 laplace2d_1loop_gcc_i500}"

for p in $PROGRAMS; do
    if [ ! -x "./$p" ]; then
        ./compilar.sh
        break
    fi
done

echo "programa,threads,rep,tempo_s" > "$OUT"

for prog in $PROGRAMS; do
    for threads in $(seq 1 "$MAX_THREADS"); do
        export OMP_NUM_THREADS="$threads"
        for rep in $(seq 1 "$REPS"); do
            saida=$("./$prog")
            tempo=$(echo "$saida" | awk '/total:/ {print $2}')
            echo "${prog},${threads},${rep},${tempo}" | tee -a "$OUT"
        done
    done
done

echo
echo "=== Media por configuracao (REPS=${REPS}) ==="
awk -F, 'NR>1 {sum[$1","$2]+=$4; cnt[$1","$2]++}
         END {print "programa,threads,tempo_medio_s";
              for (k in sum) printf "%s,%.4f\n", k, sum[k]/cnt[k]}' "$OUT" | sort -t, -k1,1 -k2,2n
