#!/bin/bash
#
# Curvas de desempenho da Tarefa 2 (Laplace 2D), de 1 ate o numero de
# nucleos FISICOS da maquina onde este script roda (detectado
# automaticamente via lscpu, ver detect_physical_cores() abaixo):
#
#   laplace2d_gcc_i50       -- exp. 1, malha 4096x4096, iter_max=50
#   laplace2d_gcc_i500      -- exp. 1, malha 4096x4096, iter_max=500
#                               (tambem baseline "loops separados" do exp. 3)
#   laplace2d_1k_gcc_i50    -- exp. 2, malha 1024x1024, iter_max=50
#   laplace2d_1k_gcc_i500   -- exp. 2, malha 1024x1024, iter_max=500
#   laplace2d_1loop_gcc_i500-- exp. 3, loops fundidos, iter_max=500
#
# Uso:
#   ./bench.sh [max_threads] [repeticoes] [arquivo_saida.csv]
#
# max_threads, se omitido, usa o numero de nucleos fisicos detectados.
#
set -e

cd "$(dirname "$0")"

detect_physical_cores() {
    if command -v lscpu >/dev/null 2>&1; then
        local cores_per_socket sockets
        cores_per_socket=$(lscpu | awk -F: '/^Core\(s\) per socket:/{gsub(/[ \t]/,"",$2); print $2}')
        sockets=$(lscpu | awk -F: '/^Socket\(s\):/{gsub(/[ \t]/,"",$2); print $2}')
        if [ -n "$cores_per_socket" ] && [ -n "$sockets" ]; then
            echo $((cores_per_socket * sockets))
            return
        fi
    fi
    # fallback: nucleos logicos (pode incluir hyperthreading -- menos preciso)
    nproc
}

detect_cpu_model() {
    if command -v lscpu >/dev/null 2>&1; then
        lscpu | awk -F: '/^Model name:/{sub(/^[ \t]+/, "", $2); print $2; exit}'
    else
        echo "CPU desconhecida"
    fi
}

PHYS_CORES="$(detect_physical_cores)"
CPU_MODEL="$(detect_cpu_model)"
MACHINE_LABEL="${CPU_MODEL} (${PHYS_CORES} nucleos fisicos)"

MAX_THREADS="${1:-$PHYS_CORES}"
REPS="${2:-5}"
OUT="${3:-resultados.csv}"

echo "Maquina detectada: $MACHINE_LABEL"
echo "OMP_NUM_THREADS testado de 1 a $MAX_THREADS, $REPS repeticoes, saida em $OUT"
echo

# avisa se o CSV de saida ja tem dados de outra maquina (ele sera sobrescrito)
if [ -s "$OUT" ]; then
    existing_machines=$(awk -F, 'NR>1 && NF>=5 {print $NF}' "$OUT" | sort -u)
    if [ -n "$existing_machines" ] && ! printf '%s\n' "$existing_machines" | grep -qF "$MACHINE_LABEL"; then
        echo "AVISO: $OUT ja contem dados de outra(s) maquina(s):" >&2
        printf '%s\n' "$existing_machines" | sed 's/^/  - /' >&2
        echo "Esta execucao (maquina: $MACHINE_LABEL) vai SOBRESCREVER esse arquivo." >&2
        echo "Para manter os dados antigos, rode com outro nome de saida, ex.:" >&2
        echo "  ./bench.sh $MAX_THREADS $REPS resultados_$(hostname 2>/dev/null || echo outra_maquina).csv" >&2
        echo >&2
    fi
fi

PROGRAMS="${PROG_LIST:-laplace2d_gcc_i50 laplace2d_gcc_i500 laplace2d_1k_gcc_i50 laplace2d_1k_gcc_i500 laplace2d_1loop_gcc_i500}"

for p in $PROGRAMS; do
    if [ ! -x "./$p" ]; then
        ./compilar.sh
        break
    fi
done

echo "programa,threads,rep,tempo_s,maquina" > "$OUT"

for prog in $PROGRAMS; do
    for threads in $(seq 1 "$MAX_THREADS"); do
        export OMP_NUM_THREADS="$threads"
        for rep in $(seq 1 "$REPS"); do
            saida=$("./$prog")
            tempo=$(echo "$saida" | awk '/total:/ {print $2}')
            echo "${prog},${threads},${rep},${tempo},${MACHINE_LABEL}" | tee -a "$OUT"
        done
    done
done

echo
echo "=== Media por configuracao (REPS=${REPS}, maquina: ${MACHINE_LABEL}) ==="
awk -F, 'NR>1 {sum[$1","$2]+=$4; cnt[$1","$2]++}
         END {print "programa,threads,tempo_medio_s";
              for (k in sum) printf "%s,%.4f\n", k, sum[k]/cnt[k]}' "$OUT" | sort -t, -k1,1 -k2,2n
