// Mesma iteracao de Jacobi 2D de jacobi_mpi_buffer.c (ver comentarios la),
// mas com a troca de colunas de borda otimizada: em vez de empacotar/
// desempacotar manualmente em buffers temporarios, usa um MPI_Type_vector
// para descrever a coluna (elementos com passo = largura da linha alocada),
// exatamente como em MPI_LAB4_2026/teste07.c. O MPI envia/recebe os
// elementos diretamente da/para a matriz local, sem copia intermediaria em
// buffer definida pela aplicacao.
//
// Uso: mpirun/srun -n 16 ./jacobi_mpi_vector

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

    // MPI_Type_vector(count, blocklen, stride, ...): "count" elementos de
    // "blocklen" doubles, separados por "stride" doubles -- aqui, rl
    // elementos de 1 double, com passo col_alloc (largura de uma linha da
    // matriz local), ou seja, exatamente uma coluna da matriz.
    MPI_Datatype coltype;
    MPI_Type_vector(rl, 1, col_alloc, MPI_DOUBLE, &coltype);
    MPI_Type_commit(&coltype);

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

    MPI_Request reqs[8];
    MPI_Status stats[8];

    if (rank == 0) {
        printf("Jacobi MPI (MPI_Type_vector, %dx%d procs) : malha global %dx%d, bloco local %dx%d, iter_max=%d\n",
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

        // ---- halo exchange: colunas, via MPI_Type_vector (sem buffer manual) ----
        MPI_Isend(&A[1][1],  1, coltype, nbrs[LEFT],  TAG_TO_LEFT,  cartcomm, &reqs[2]);
        MPI_Isend(&A[1][cl], 1, coltype, nbrs[RIGHT], TAG_TO_RIGHT, cartcomm, &reqs[3]);
        MPI_Irecv(&A[1][0],      1, coltype, nbrs[LEFT],  TAG_TO_RIGHT, cartcomm, &reqs[6]);
        MPI_Irecv(&A[1][cl + 1], 1, coltype, nbrs[RIGHT], TAG_TO_LEFT,  cartcomm, &reqs[7]);

        MPI_Waitall(8, reqs, stats);

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
        printf("config=mpi_vector procs=%d malha=%dx%d iter_max=%d tempo_s=%f\n",
               numtasks, N, M, ITER_MAX, tempo);
    }

    MPI_Type_free(&coltype);
    free(A[0]); free(A);
    free(Anew[0]); free(Anew);

    MPI_Finalize();
    return 0;
}
