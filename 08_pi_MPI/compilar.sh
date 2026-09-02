#!/bin/bash
#
# Compilacao das 3 versoes do calculo de Pi no SDumont (mesmo toolchain
# Intel PSXE usado em MPI_LAB4_2026/compilar.sh).

module load intel_psxe/2020_sequana

icc    -qopenmp pi_openmp.c -o pi_openmp
mpiicc            pi_mpi.c    -o pi_mpi
mpiicc -qopenmp pi_hybrid.c -o pi_hybrid

echo "  **** COMPILACAO COMPLETA ****  "
