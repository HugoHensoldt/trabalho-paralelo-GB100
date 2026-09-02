#!/bin/bash
#
# Compilacao das 2 variantes do Jacobi MPI no SDumont (mesmo toolchain Intel
# PSXE usado em MPI_LAB4_2026/compilar.sh e 08_pi_MPI/compilar.sh).

module load intel_psxe/2020_sequana

mpiicc -O2 jacobi_mpi_buffer.c -o jacobi_mpi_buffer
mpiicc -O2 jacobi_mpi_vector.c -o jacobi_mpi_vector

echo "  **** COMPILACAO COMPLETA ****  "
