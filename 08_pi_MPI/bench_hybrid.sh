#!/bin/bash
#SBATCH --nodes=2                      #Numero de Nós
#SBATCH --ntasks-per-node=1            #1 processo MPI por nó
#SBATCH --ntasks=2                     #2 processos MPI no total
#SBATCH --cpus-per-task=6              #Nucleos internos disponiveis para as threads de cada processo
#SBATCH -p sequana_cpu_dev             #Fila (partition) a ser utilizada
#SBATCH -J pi-hibrido                  #Nome job
#SBATCH --exclusive                    #Utilização exclusiva dos nós durante a execução do job

####################################################
#
# Hibrido MPI + OpenMP (2 processos, um por no do
# SDumont, threads nos nucleos internos): varia
# OMP_NUM_THREADS de 1 a 6 e mede o tempo medio de 30
# execucoes independentes por configuracao.
#
# USAGE: sbatch bench_hybrid.sh
#
####################################################

module load intel_psxe/2020_sequana

cd "$SLURM_SUBMIT_DIR"

N=2000000000
REPS=30
EXEC=./pi_hybrid
OUT=resultados_hibrido.csv

echo "threads,rep,pi,tempo_s" > "$OUT"

for T in 1 2 3 4 5 6; do
    export OMP_NUM_THREADS=$T
    for rep in $(seq 1 $REPS); do
        LINHA=$(srun -n 2 --cpus-per-task=$T $EXEC $N)
        PI=$(echo "$LINHA"    | grep -oE 'pi=[0-9.]+'      | cut -d= -f2)
        TEMPO=$(echo "$LINHA" | grep -oE 'tempo_s=[0-9.]+' | cut -d= -f2)
        echo "${T},${rep},${PI},${TEMPO}" | tee -a "$OUT"
    done
done

echo
echo "=== Media por numero de threads (N=${N}, REPS=${REPS}, 2 processos) ==="
awk -F, 'NR>1 {sum[$1]+=$4; cnt[$1]++}
         END {print "threads,tempo_medio_s";
              for (k in sum) printf "%s,%.6f\n", k, sum[k]/cnt[k]}' "$OUT" | sort -t, -k1,1n
