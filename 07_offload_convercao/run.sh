#!/bin/bash
#SBATCH --nodes=1                      #Numero de Nós
#SBATCH --ntasks-per-node=1            #Numero de tarefas por Nó
#SBATCH --cpus-per-task=1              #Numero de threads (só o host lança o kernel)
#SBATCH --gres=gpu:1                   #Numero de GPUs solicitadas
#SBATCH -p sequana_gpu_dev             #Fila (partition) com GPU no Santos Dumont (ajuste se sinfo mostrar outro nome)
#SBATCH -J semana-sdumont-cuda         #Nome job

####################################################
#
# USAGE: sbatch run.sh EXECUTAVEL
#
####################################################

module load cuda

EXEC=${1}

srun -N 1 ${EXEC}
