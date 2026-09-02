#!/bin/bash
#SBATCH --nodes=2                      #Numero de Nós
#SBATCH --ntasks-per-node=8            #Numero de tarefas por Nó
#SBATCH --ntasks=16                    #Numero total de tarefas MPI (grid cartesiano 4x4)
#SBATCH -p sequana_cpu_dev             #Fila (partition) a ser utilizada
#SBATCH -J jacobi-mpi                  #Nome job
#SBATCH --exclusive                    #Utilização exclusiva dos nós durante a execução do job

#############################################
#   Baseado em MPI_LAB4_2026/sub_teste11.sh #
#   (mesmo numero de processos/nos).        #
#-------------------------------------------#
#   Para a submissão na Sdumont:            #
#   sbatch sub.sh EXECUTAVEL                #
#   ex.: sbatch sub.sh ./jacobi_mpi_buffer   #
#        sbatch sub.sh ./jacobi_mpi_vector   #
#-------------------------------------------#
#   Para a visualização dos Jobs:           #
#   squeue -u $USER                         #
#-------------------------------------------#
#   Para cancelar o Job:                    #
#   scancel NUMERO_DO_JOB                   #
#############################################

module load intel_psxe/2020_sequana

EXEC=${1}

srun -n $SLURM_NTASKS $EXEC
