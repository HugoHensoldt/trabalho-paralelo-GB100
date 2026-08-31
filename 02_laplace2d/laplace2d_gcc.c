/*
 *  Baseado em jacobi/laplace2d.c (Copyright 2012 NVIDIA Corporation,
 *  Apache License 2.0). Versao gcc sem calculo de erro: o `while` passa a
 *  ser controlado só por iter_max (o calculo de error/tol foi removido, ou
 *  seja, a regiao sequencial de cada iteracao fica menor e fixa, so
 *  iter++ / printf condicional).
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
#define ITER_MAX 50
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

    printf("Jacobi (sem erro) gcc: %d x %d mesh, iter_max=%d\n", n, m, iter_max);

    StartTimer();
    int iter = 0;

    while ( iter < iter_max )    // sem tol/error: condicao de parada e so iter_max (removida em rel. ao original)
    {
// fork/join #1: reparte o laco em j entre as threads
#pragma omp parallel for shared(Anew, A)
        for( int j = 1; j < n-1; j++)
        {
            for( int i = 1; i < m-1; i++ )
            {
                Anew[j][i] = 0.25 * ( A[j][i+1] + A[j][i-1]
                                    + A[j-1][i] + A[j+1][i]);
                // error = fmax( error, fabs(Anew[j][i] - A[j][i])); // comentado: corrida em 'error' (shared sem reduction/atomic)
            }
        }

// fork/join #2: nova regiao paralela so para copiar Anew -> A
#pragma omp parallel for shared(Anew, A)
        for( int j = 1; j < n-1; j++)
        {
            for( int i = 1; i < m-1; i++ )
            {
                A[j][i] = Anew[j][i];
            }
        }

        if(iter % 100 == 0) printf("%5d\n", iter);

        iter++;
    }

    double runtime = GetTimer();

    printf(" total: %f s\n", runtime / 1000);
}
