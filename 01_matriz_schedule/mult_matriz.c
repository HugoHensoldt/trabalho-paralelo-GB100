/*
 * Multiplicacao de matrizes C = A x B (N x N). Diferenca em relacao aos
 * exemplos de exemplo_4/ (que so tinham lacos triviais de printf): aqui o
 * laco usa ordem i-k-j em vez de i-j-k, para que B[k][j] e C[i][j] sejam
 * acessados com passo 1 no laco mais interno (cache-friendly). Isso e so
 * uma otimizacao de acesso a memoria, nao envolve OpenMP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifndef N
#define N 4096
#endif

static double A[N][N], B[N][N], C[N][N];

static const char *sched_name(omp_sched_t kind) {
    switch (kind & ~omp_sched_monotonic) {
        case omp_sched_static:  return "static";
        case omp_sched_dynamic: return "dynamic";
        case omp_sched_guided:  return "guided";
        case omp_sched_auto:    return "auto";
        default:                return "desconhecido";
    }
}

int main(void) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (double)((i + j) % 100) / 10.0;
            B[i][j] = (double)((i - j + N) % 100) / 10.0;
            C[i][j] = 0.0;
        }
    }

    int nthreads = 0;
    double t0 = omp_get_wtime();

    // Mesma diretiva `#pragma omp parallel` dos exemplos 4 (cria a equipe de
    // threads). Criar threads tem overhead alto (fora do loop) 
    #pragma omp parallel
    {
        // SINGLE: so uma thread executa omp_get_num_threads()
        #pragma omp single
        nthreads = omp_get_num_threads();

        // (modo, chunk) de OMP_SCHEDULE em cada execucao, sem recompilar (ver run_benchmark.sh).

        #pragma omp for schedule(runtime)
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                double a = A[i][k];
                for (int j = 0; j < N; j++) {
                    C[i][j] += a * B[k][j];
                }
            }
        }
    }

    double t1 = omp_get_wtime();

    double checksum = 0.0;
    for (int i = 0; i < N; i += N / 8) {
        for (int j = 0; j < N; j += N / 8) {
            checksum += C[i][j];
        }
    }

    // printf dos tempos e chunks
    omp_sched_t kind;
    int chunk;
    omp_get_schedule(&kind, &chunk);

    printf("N=%d threads=%d schedule=%s chunk=%d tempo_s=%.4f checksum=%.6f\n",
           N, nthreads, sched_name(kind), chunk, t1 - t0, checksum);

    return 0;
}
