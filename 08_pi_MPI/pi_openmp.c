// Calculo de Pi por integracao numerica (regra do ponto medio), mesma
// formula de MPI_LAB4_2026/teste04.c, paralelizado com OpenMP puro
// (1 no computacional, threads sobre os nucleos internos).
//
// Uso: ./pi_openmp N
//   N = numero de intervalos da integracao (ex.: 2000000000)
// OMP_NUM_THREADS controla o numero de threads (nao passado por argv para
// manter o mesmo padrao dos demais laboratorios: ver README.md).

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s N\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);
    double h = 1.0 / (double)n;
    double sum = 0.0;

    double ini = omp_get_wtime(); // CRONOMETRAGEM: marca o tempo inicial

    // PARALELIZACAO OPENMP: cria as threads (OMP_NUM_THREADS) e reparte as
    // iteracoes do laco entre elas (schedule static = fatias iguais).
    // reduction(+:sum) da uma copia privada de "sum" pra cada thread e soma
    // tudo no final, sem precisar de lock/critical.
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (long long i = 1; i <= n; i++) {
        // PONTO MEDIO: valor de 4/(1+x^2) no meio do i-esimo subintervalo
        double x = h * ((double)i - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    double pi = h * sum;
    double fim = omp_get_wtime(); // CRONOMETRAGEM: marca o tempo final

    printf("config=openmp threads=%d n=%lld pi=%.16f tempo_s=%f\n",
           omp_get_max_threads(), n, pi, fim - ini); // threads = quantas threads OpenMP rodaram de fato

    return 0;
}
