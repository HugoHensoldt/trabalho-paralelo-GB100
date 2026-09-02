// Calculo de Pi por integracao numerica (regra do ponto medio), hibrido
// MPI + OpenMP: 2 processos MPI (um por no do SDumont) e, dentro de cada
// processo, threads OpenMP paralelizando a fatia local nos nucleos internos
// do no.
//
// Cada rank MPI recebe a mesma fatia entrelacada de indices que pi_mpi.c
// (i = myid+1, myid+1+numprocs, ...); a diferenca e que essa fatia local e
// paralelizada com #pragma omp parallel for em vez de somada
// sequencialmente, permitindo comparar diretamente com pi_mpi.c (mesmo
// particionamento entre processos) e pi_openmp.c (mesmo paralelismo de
// threads, um nivel abaixo).
//
// Uso: mpirun/srun -n 2 ./pi_hybrid N
//   N = numero de intervalos da integracao (ex.: 2000000000)
// OMP_NUM_THREADS controla o numero de threads por processo MPI.

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int myid, numprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (argc < 2) {
        if (myid == 0) fprintf(stderr, "uso: %s N\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    long long n = atoll(argv[1]);
    MPI_Bcast(&n, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    double h = 1.0 / (double)n;
    double sum = 0.0;

    // Numero de indices i = myid+1, myid+1+numprocs, ... <= n atribuidos a
    // este rank (mesma fatia entrelacada usada em pi_mpi.c).
    long long local_n = (n - myid - 1) / numprocs + 1;
    if (n < myid + 1) local_n = 0;

    MPI_Barrier(MPI_COMM_WORLD);
    double ini = MPI_Wtime();

    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (long long j = 0; j < local_n; j++) {
        long long i = myid + 1 + j * numprocs;
        double x = h * ((double)i - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    double mypi = h * sum;
    double pi = 0.0;
    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double fim_local = MPI_Wtime() - ini;
    double tempo;
    MPI_Reduce(&fim_local, &tempo, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (myid == 0) {
        printf("config=hybrid procs=%d threads=%d n=%lld pi=%.16f tempo_s=%f\n",
               numprocs, omp_get_max_threads(), n, pi, tempo);
    }

    MPI_Finalize();
    return 0;
}
