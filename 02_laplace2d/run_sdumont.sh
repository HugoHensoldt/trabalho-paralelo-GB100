#!/bin/bash
#SBATCH --nodes=1                      #Numero de Nós
#SBATCH --ntasks=1                     #Numero total de tarefas MPI (so 1: e OpenMP, nao MPI)
#SBATCH --cpus-per-task=48             #Nucleos reservados p/ as threads (confirmado via sinfo -p sequana_cpu_dev: CPUS=48)
#SBATCH -p sequana_cpu_dev             #Fila (partition) a ser utilizada
#SBATCH -J laplace2d-bench             #Nome job
#SBATCH --exclusive                    #Uso exclusivo do no (evita ruido de outros jobs na medicao)
#SBATCH --time=00:15:00                #so os 3 programas rapidos (iter_max=50 e 1k_i500); ~250s medido no smoke test + folga

####################################################
#
# USAGE: sbatch run_sdumont.sh
#
# Roda o benchmark completo do Laplace 2D (02_laplace2d) no Santos Dumont.
# IMPORTANTE: o numero de threads testado (MAX_THREADS abaixo) deve bater
# com --cpus-per-task acima -- nao usamos deteccao automatica via lscpu
# aqui porque, dentro de um job SLURM, o lscpu costuma reportar os
# nucleos FISICOS DO NO INTEIRO, nao so os reservados para este job pelo
# cgroup. Se --cpus-per-task mudar, mude MAX_THREADS junto.
#
####################################################

cd "$SLURM_SUBMIT_DIR"

if [ ! -x ./laplace2d_gcc_i50 ]; then
    bash compilar.sh
fi

PROG_LIST='laplace2d_gcc_i50 laplace2d_1k_gcc_i50 laplace2d_1k_gcc_i500' \
    bash bench.sh 48 5 resultados_sdumont.csv
