/*
 *  Baseado em jacobi/laplace2d.c (Copyright 2012 NVIDIA Corporation,
 *  Apache License 2.0). Funde os dois `#pragma omp parallel for` de cada
 *  iteracao (laplace2d_gcc.c) em uma unica regiao `#pragma omp parallel`
 *  com dois `#pragma omp for` dentro -- so um fork/join de threads por
 *  iteracao do `while`, em vez de dois. A barreira implicita ao final do
 *  primeiro `#pragma omp for` garante que Anew esta completo antes do laco
 *  de copia comecar (correcao preservada). O calculo de erro fica comentado
 *  (ver nota no README/APRESENTACAO sobre a condicao de corrida).
 */

#include <string.h>
#include <omp.h>
#include "timer.h"
#include <stdio.h>

#ifndef NN
#define NN 4096
#endif
#ifndef NM
#define NM 4096
#endif
#ifndef ITER_MAX
#define ITER_MAX 500
#endif

double A[NN][NM];
double Anew[NN][NM];

int main(int argc, char** argv)
{
    const int n = NN;
    const int m = NM;
    const int iter_max = ITER_MAX;

    memset(A, 0, n * m * sizeof(double));
    memset(Anew, 0, n * m * sizeof(double));

    for (int j = 0; j < n; j++)
    {
        A[j][0]    = 1.0;
        Anew[j][0] = 1.0;
    }

    printf("Jacobi (loops fundidos) gcc: %d x %d mesh, iter_max=%d\n", n, m, iter_max);

    StartTimer();
    int iter = 0;

    while ( iter < iter_max )    // sem tol/error: condicao de parada e so iter_max
    {
// fork/join UNICO por iteracao (laplace2d_gcc.c abre dois, um por laco `for`)
#pragma omp parallel shared(Anew, A)
        {
// so reparte o trabalho dentro do time ja criado acima, sem novo fork/join
#pragma omp for
            for( int j = 1; j < n-1; j++)
            {
                for( int i = 1; i < m-1; i++ )
                {
                    Anew[j][i] = 0.25 * ( A[j][i+1] + A[j][i-1]
                                        + A[j-1][i] + A[j+1][i]);
                    // error = fmax( error, fabs(Anew[j][i] - A[j][i])); // comentado: corrida em 'error' (shared sem reduction/atomic)
                }
            }

// barreira implicita ao fim do "for" acima garante Anew completo antes daqui
#pragma omp for
            for( int j = 1; j < n-1; j++)
            {
                for( int i = 1; i < m-1; i++ )
                {
                    A[j][i] = Anew[j][i];
                }
            }
        }

        if(iter % 100 == 0) printf("%5d\n", iter);

        iter++;
    }

    double runtime = GetTimer();

    printf(" total: %f s\n", runtime / 1000);
}
