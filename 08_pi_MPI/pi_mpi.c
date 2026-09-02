// Calculo de Pi por integracao numerica (regra do ponto medio) com MPI puro,
// baseado em MPI_LAB4_2026/teste04.c e calc_pi_MPI.c (2 nos computacionais
// do SDumont, um processo por core alocado, sem threads OpenMP).
//
// Nota: calc_pi_MPI.c tem um bug de chaveamento -- MPI_Reduce, MPI_Wtime()
// final e o printf ficam, por falta de chaves, dentro do laco `for` e do
// `if (n != 0)`, entao so sao executados quando a ULTIMA iteracao do laco
// roda (e nunca, se n == 0). Esta versao corrige isso: cada rank soma sua
// fatia inteira do somatorio e so entao participa do Reduce, uma vez.
//
// Uso: mpirun/srun -n P ./pi_mpi N
//   N = numero de intervalos da integracao (ex.: 2000000000)

#include <mpi.h>
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

    MPI_Barrier(MPI_COMM_WORLD);
    double ini = MPI_Wtime();

    // Decomposicao entrelacada: cada rank cuida dos indices
    // myid+1, myid+1+numprocs, myid+1+2*numprocs, ...
    for (long long i = myid + 1; i <= n; i += numprocs) {
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
        printf("config=mpi procs=%d n=%lld pi=%.16f tempo_s=%f\n",
               numprocs, n, pi, tempo);
    }

    MPI_Finalize();
    return 0;
}
