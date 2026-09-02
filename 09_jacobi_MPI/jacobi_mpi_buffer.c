// Iteracao de Jacobi 2D (mesma equacao/condicoes de contorno de
// 02_laplace2d/laplace2d_gcc.c) paralelizada com MPI puro por decomposicao
// de dominio, usando a topologia cartesiana 4x4 de MPI_LAB4_2026/teste11.c
// como base: 16 processos, cada um dono de um bloco 1024x1024 da malha
// global 4096x4096.
//
// Halo exchange (troca de bordas) no inicio de cada iteracao, com
// MPI_Isend/MPI_Irecv (nao bloqueante) para os 4 vizinhos diretos
// (cima/baixo/esquerda/direita):
//   - linhas de borda: contiguas na memoria, enviadas diretamente;
//   - colunas de borda: empacotadas em buffers temporarios antes do envio e
//     desempacotadas na posicao correspondente ao receber (ver a variante
//     jacobi_mpi_vector.c, que troca esse empacotamento manual por um
//     MPI_Type_vector, como em MPI_LAB4_2026/teste07.c).
//
// Processos nas bordas do dominio global tem vizinho MPI_PROC_NULL naquela
// direcao (MPI_Cart_shift com periods=0, igual a teste11.c) -- o
// Isend/Irecv correspondente vira um no-op, e a condicao de contorno de
// Dirichlet fixada na inicializacao (coluna global 0 = 1.0, demais bordas
// = 0.0) nunca e sobrescrita.
//
// Uso: mpirun/srun -n 16 ./jacobi_mpi_buffer

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define PY 4
#define PX 4
#define SIZE (PY * PX)

#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#define TAG_TO_UP    0
#define TAG_TO_DOWN  1
#define TAG_TO_LEFT  2
#define TAG_TO_RIGHT 3

#ifndef N
#define N 4096
#endif
#ifndef M
#define M 4096
#endif
#ifndef ITER_MAX
#define ITER_MAX 50
#endif

static double **aloca_matriz(int linhas, int colunas)
{
    double **mat = malloc(linhas * sizeof(double *));
    mat[0] = calloc((size_t)linhas * colunas, sizeof(double));
    for (int j = 1; j < linhas; j++)
        mat[j] = mat[0] + (size_t)j * colunas;
    return mat;
}

int main(int argc, char *argv[])
{
    int numtasks, rank;
    int dims[2] = {PY, PX}, periods[2] = {0, 0}, reorder = 0, coords[2];
    int nbrs[4];
    MPI_Comm cartcomm;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (numtasks != SIZE) {
        if (rank == 0)
            fprintf(stderr, "Must specify %d processos (4x4). Terminating.\n", SIZE);
        MPI_Finalize();
        return 1;
    }

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm);
    MPI_Comm_rank(cartcomm, &rank);
    MPI_Cart_coords(cartcomm, rank, 2, coords);
    MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
    MPI_Cart_shift(cartcomm, 1, 1, &nbrs[LEFT], &nbrs[RIGHT]);

    const int rl = N / PY;   // linhas reais do bloco local
    const int cl = M / PX;   // colunas reais do bloco local
    const int lin_alloc = rl + 2;  // + 2 linhas fantasma (cima/baixo)
    const int col_alloc = cl + 2;  // + 2 colunas fantasma (esquerda/direita)

    double **A    = aloca_matriz(lin_alloc, col_alloc);
    double **Anew = aloca_matriz(lin_alloc, col_alloc);

    // Condicao de contorno de Dirichlet: coluna global 0 = 1.0 (demais = 0.0,
    // ja zeradas pelo calloc). So os processos na 1a coluna do grid cartesiano
    // possuem essa borda.
    if (coords[1] == 0) {
        for (int j = 0; j <= rl + 1; j++) {
            A[j][1]    = 1.0;
            Anew[j][1] = 1.0;
        }
    }

    // Faixa de atualizacao do stencil: exclui a borda global de Dirichlet
    // (nunca recalculada), so processos nas bordas do grid cartesiano tem
    // faixa reduzida.
    const int jlo = (coords[0] == 0)      ? 2      : 1;
    const int jhi = (coords[0] == PY - 1) ? rl - 1 : rl;
    const int ilo = (coords[1] == 0)      ? 2      : 1;
    const int ihi = (coords[1] == PX - 1) ? cl - 1 : cl;

    double *sendbuf_l = malloc(rl * sizeof(double));
    double *sendbuf_r = malloc(rl * sizeof(double));
    double *recvbuf_l = malloc(rl * sizeof(double));
    double *recvbuf_r = malloc(rl * sizeof(double));

    MPI_Request reqs[8];
    MPI_Status stats[8];

    if (rank == 0) {
        printf("Jacobi MPI (buffer, %dx%d procs) : malha global %dx%d, bloco local %dx%d, iter_max=%d\n",
               PY, PX, N, M, rl, cl, ITER_MAX);
    }

    MPI_Barrier(cartcomm);
    double ini = MPI_Wtime();

    for (int iter = 0; iter < ITER_MAX; iter++) {

        // ---- halo exchange: linhas (contiguas, envio direto) ----
        MPI_Isend(&A[1][1],  cl, MPI_DOUBLE, nbrs[UP],   TAG_TO_UP,   cartcomm, &reqs[0]);
        MPI_Isend(&A[rl][1], cl, MPI_DOUBLE, nbrs[DOWN], TAG_TO_DOWN, cartcomm, &reqs[1]);
        MPI_Irecv(&A[0][1],      cl, MPI_DOUBLE, nbrs[UP],   TAG_TO_DOWN, cartcomm, &reqs[4]);
        MPI_Irecv(&A[rl + 1][1], cl, MPI_DOUBLE, nbrs[DOWN], TAG_TO_UP,   cartcomm, &reqs[5]);

        // ---- halo exchange: colunas (empacotadas em buffer temporario) ----
        for (int j = 1; j <= rl; j++) {
            sendbuf_l[j - 1] = A[j][1];
            sendbuf_r[j - 1] = A[j][cl];
        }
        MPI_Isend(sendbuf_l, rl, MPI_DOUBLE, nbrs[LEFT],  TAG_TO_LEFT,  cartcomm, &reqs[2]);
        MPI_Isend(sendbuf_r, rl, MPI_DOUBLE, nbrs[RIGHT], TAG_TO_RIGHT, cartcomm, &reqs[3]);
        MPI_Irecv(recvbuf_l, rl, MPI_DOUBLE, nbrs[LEFT],  TAG_TO_RIGHT, cartcomm, &reqs[6]);
        MPI_Irecv(recvbuf_r, rl, MPI_DOUBLE, nbrs[RIGHT], TAG_TO_LEFT,  cartcomm, &reqs[7]);

        MPI_Waitall(8, reqs, stats);

        // desempacota as colunas recebidas para as colunas fantasma
        for (int j = 1; j <= rl; j++) {
            A[j][0]      = recvbuf_l[j - 1];
            A[j][cl + 1] = recvbuf_r[j - 1];
        }

        // ---- stencil de 5 pontos sobre a faixa de atualizacao local ----
        for (int j = jlo; j <= jhi; j++) {
            for (int i = ilo; i <= ihi; i++) {
                Anew[j][i] = 0.25 * (A[j][i + 1] + A[j][i - 1]
                                    + A[j - 1][i] + A[j + 1][i]);
            }
        }

        for (int j = jlo; j <= jhi; j++) {
            for (int i = ilo; i <= ihi; i++) {
                A[j][i] = Anew[j][i];
            }
        }
    }

    double fim_local = MPI_Wtime() - ini;
    double tempo;
    MPI_Reduce(&fim_local, &tempo, 1, MPI_DOUBLE, MPI_MAX, 0, cartcomm);

    if (rank == 0) {
        printf("config=mpi_buffer procs=%d malha=%dx%d iter_max=%d tempo_s=%f\n",
               numtasks, N, M, ITER_MAX, tempo);
    }

    free(sendbuf_l); free(sendbuf_r); free(recvbuf_l); free(recvbuf_r);
    free(A[0]); free(A);
    free(Anew[0]); free(Anew);

    MPI_Finalize();
    return 0;
}
