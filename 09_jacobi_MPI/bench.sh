#!/bin/bash
#SBATCH --nodes=2                      #Numero de Nós
#SBATCH --ntasks-per-node=8            #Numero de tarefas por Nó
#SBATCH --ntasks=16                    #Numero total de tarefas MPI (grid cartesiano 4x4)
#SBATCH -p sequana_cpu_dev             #Fila (partition) a ser utilizada
#SBATCH -J jacobi-bench                #Nome job
#SBATCH --exclusive                    #Utilização exclusiva dos nós durante a execução do job

####################################################
#
# Compara as 2 variantes de halo exchange (buffer manual
# x MPI_Type_vector) do Jacobi MPI 4096x4096 / 16 processos
# (grid cartesiano 4x4), medindo o tempo medio de REPS
# execucoes independentes de cada uma.
#
# USAGE: sbatch bench.sh
#
####################################################

module load intel_psxe/2020_sequana

cd "$SLURM_SUBMIT_DIR"

REPS=10
OUT=resultados.csv

echo "config,rep,tempo_s" > "$OUT"

for EXEC in ./jacobi_mpi_buffer ./jacobi_mpi_vector; do
    CFG=$(basename "$EXEC")
    for rep in $(seq 1 $REPS); do
        LINHA=$(srun -n $SLURM_NTASKS $EXEC)
        TEMPO=$(echo "$LINHA" | grep -oE 'tempo_s=[0-9.]+' | cut -d= -f2)
        echo "${CFG},${rep},${TEMPO}" | tee -a "$OUT"
    done
done

echo
echo "=== Media por configuracao (REPS=${REPS}) ==="
awk -F, 'NR>1 {sum[$1]+=$3; cnt[$1]++}
         END {print "config,tempo_medio_s";
              for (k in sum) printf "%s,%.6f\n", k, sum[k]/cnt[k]}' "$OUT"
