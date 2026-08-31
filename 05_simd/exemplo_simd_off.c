#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

///////////////////////////////////////////////
//   LACO EM FAIXAS (STRIP-MINING) SEM SIMD
///////////////////////////////////////////////

const int STRIP_SIZE = 128; // Multiplo do comprimento do vetor SIMD

// Trabalho por elemento: uma recorrencia simples aplicada varias vezes.
// A repeticao aumenta a quantidade de calculo por elemento lido/escrito,
// para que o teste meca poder de processamento (onde o SIMD ajuda) em vez
// de ficar limitado pela banda de memoria.
#define REPETICOES 50

double DoSomeWork(double x)
{
    for (int k = 0; k < REPETICOES; k++)
        x = x*0.99 + 1.0;
    return x;
}

int main(int argc, char *argv[])
{
    int n = atoi(argv[1]);
    double *A = (double*) malloc(n * sizeof(double));

    for (int i = 0; i < n; i++)
        A[i] = (double)(i % 1000) / 100.0;

    int nTrunc = n - n % STRIP_SIZE; // Multiplo do comprimento do vetor SIMD

    double start = omp_get_wtime();

    #pragma omp parallel for
    for (int ii = 0; ii < nTrunc; ii += STRIP_SIZE) // Paralelismo de threads (externo)
        for (int i = ii; i < ii + STRIP_SIZE; i++)  // SEM "#pragma omp simd" aqui
            A[i] = DoSomeWork(A[i]);

    // Laco remanescente
    for (int i = nTrunc; i < n; i++)
        A[i] = DoSomeWork(A[i]);

    double end = omp_get_wtime();

    double soma = 0.0;
    for (int i = 0; i < n; i++)
        soma += A[i];

    printf("versao = sem_simd, n = %d, tempo = %lf, soma = %lf\n", n, end - start, soma);

    free(A);
    return 0;
}
