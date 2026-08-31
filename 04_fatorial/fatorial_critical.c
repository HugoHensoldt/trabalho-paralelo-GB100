#include <stdio.h>
#include <omp.h>

int main() {
    long long produto;
    int n = 10;
    int reps = 100000;
    double start, end;

    start = omp_get_wtime(); // marca o inicio da medicao 
    // reps: repete o calculo inteiro 100000x so para acumular tempo mensuravel,  n=10 e' fixo
    for (int r = 0; r < reps; r++) {
        produto = 1;
        #pragma omp parallel for // divide as 10 iteracoes entre as threads
        for (int i = 1; i <= n; i++) {
            #pragma omp critical // trava a cada iteracao: so uma thread multiplica por vez 
            produto *= i;
        }
    }
    end = omp_get_wtime(); // marca o fim da medicao

    printf("Fatorial de %d: %lld\n", n, produto);
    printf("Tempo decorrido: %lf\n", end - start);
    return 0;
}
