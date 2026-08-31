#!/bin/bash
#SBATCH --nodes=1                      #Numero de Nós
#SBATCH --ntasks-per-node=1            #Numero de tarefas por Nó
#SBATCH --ntasks=1                     #Numero total de tarefas MPI
#SBATCH --cpus-per-task=24             #Numero de threads
#SBATCH --gres=gpu:1                   #Numero de GPUs solicitadas
#SBATCH -p treinamento                 #Fila (partition) a ser utilizada (verificar fila com GPU no cluster)
#SBATCH -J semana-sdumont-offload      #Nome job

####################################################
#
# USAGE: sbatch run.sh EXECUTABLE
#
####################################################

module load nvhpc

EXEC=${1}

srun -N 1 ${EXEC}
