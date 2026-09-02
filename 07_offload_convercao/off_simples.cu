#include <stdio.h>
#include <cuda_runtime.h>

/* off_simples.cu — versão CUDA de off_simples.c (soma de vetores) */

__global__ void soma(int *a, int *b, int *c, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N)
        c[i] = a[i] + b[i];
}

int main(int argc, char *argv[]) {
    int N = 1000;
    size_t bytes = N * sizeof(int);

    // Vetores no hospedeiro (host)
    int *a = (int *)malloc(bytes);
    int *b = (int *)malloc(bytes);
    int *c = (int *)malloc(bytes);

    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }

    // Alocando e copiando os vetores para o dispositivo (device/GPU)
    int *d_a, *d_b, *d_c;
    cudaMalloc((void **)&d_a, bytes);
    cudaMalloc((void **)&d_b, bytes);
    cudaMalloc((void **)&d_c, bytes);

    cudaMemcpy(d_a, a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, bytes, cudaMemcpyHostToDevice);

    // Kernel executado no dispositivo, equivalente à região "target"
    int threadsPorBloco = 256;
    int blocos = (N + threadsPorBloco - 1) / threadsPorBloco;
    soma<<<blocos, threadsPorBloco>>>(d_a, d_b, d_c, N);

    // Copiando o resultado de volta para o hospedeiro
    cudaMemcpy(c, d_c, bytes, cudaMemcpyDeviceToHost);

    // Exibindo os resultados
    for (int i = 0; i < 10; i++)
        printf("%d ", c[i]);
    printf("\n");

    cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);
    free(a); free(b); free(c);
    return 0;
}
