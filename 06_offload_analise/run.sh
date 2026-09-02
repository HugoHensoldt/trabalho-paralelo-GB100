#!/bin/bash
#SBATCH --nodes=1                      #Numero de Nós
#SBATCH --ntasks-per-node=1            #Numero de tarefas por Nó
#SBATCH --ntasks=1                     #Numero total de tarefas MPI
#SBATCH --cpus-per-task=24             #Numero de threads
#SBATCH --gres=gpu:1                   #Numero de GPUs solicitadas
#SBATCH -p sequana_gpu_dev              #Fila (partition) com GPU no Santos Dumont (ajuste se sinfo mostrar outro nome)
#SBATCH -J semana-sdumont-offload      #Nome job

####################################################
#
# USAGE: sbatch run.sh EXECUTABLE
#
####################################################

module load nvhpc

EXEC=${1}

srun -N 1 ${EXEC}
