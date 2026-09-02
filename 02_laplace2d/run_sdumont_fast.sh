#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH -p sequana_cpu_dev
#SBATCH -J laplace2d-fast
#SBATCH --exclusive
#SBATCH --time=00:15:00

cd "$SLURM_SUBMIT_DIR"
if [ ! -x ./laplace2d_gcc_i50 ]; then
    bash compilar.sh
fi
PROG_LIST='laplace2d_gcc_i50 laplace2d_1k_gcc_i50 laplace2d_1k_gcc_i500' \
    bash bench.sh 48 5 resultados_sdumont.csv
