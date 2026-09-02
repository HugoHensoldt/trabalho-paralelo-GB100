#!/bin/bash
#SBATCH --nodes=2                      #Numero de Nós
#SBATCH --ntasks-per-node=3            #Numero de tarefas por Nó (ate 6 processos no total)
#SBATCH --ntasks=6                     #Numero total de tarefas MPI
#SBATCH -p sequana_cpu_dev             #Fila (partition) a ser utilizada
#SBATCH -J pi-mpi                      #Nome job
#SBATCH --exclusive                    #Utilização exclusiva dos nós durante a execução do job

####################################################
#
# MPI puro (2 nos do SDumont): varia o numero de
# processos de 1 a 6 e mede o tempo medio de 30
# execucoes independentes por configuracao.
#
# "-m cyclic" distribui os processos alternando entre
# os 2 nos alocados (em vez de preencher o 1o no
# primeiro), para que mesmo com poucos processos (ex.:
# P=2) o trabalho fique de fato espalhado pelos 2 nos.
#
# USAGE: sbatch bench_mpi.sh
#
####################################################

module load intel_psxe/2020_sequana

cd "$SLURM_SUBMIT_DIR"

N=2000000000
REPS=30
EXEC=./pi_mpi
OUT=resultados_mpi.csv

echo "procs,rep,pi,tempo_s" > "$OUT"

for P in 1 2 3 4 5 6; do
    for rep in $(seq 1 $REPS); do
        LINHA=$(srun -n $P -m cyclic $EXEC $N)
        PI=$(echo "$LINHA"    | grep -oE 'pi=[0-9.]+'      | cut -d= -f2)
        TEMPO=$(echo "$LINHA" | grep -oE 'tempo_s=[0-9.]+' | cut -d= -f2)
        echo "${P},${rep},${PI},${TEMPO}" | tee -a "$OUT"
    done
done

echo
echo "=== Media por numero de processos (N=${N}, REPS=${REPS}) ==="
awk -F, 'NR>1 {sum[$1]+=$4; cnt[$1]++}
         END {print "procs,tempo_medio_s";
              for (k in sum) printf "%s,%.6f\n", k, sum[k]/cnt[k]}' "$OUT" | sort -t, -k1,1n
