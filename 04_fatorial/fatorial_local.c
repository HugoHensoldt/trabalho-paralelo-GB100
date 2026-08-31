#include <stdio.h>
#include <omp.h>

int main() {
    long long produto;
    int n = 10;
    int reps = 100000;
    double start, end;

    start = omp_get_wtime(); // marca o inicio da medicao
    // reps: repete o calculo inteiro 100000x so para acumular tempo mensuravel, n=10 e' fixo
    for (int r = 0; r < reps; r++) {
        produto = 1;
        #pragma omp parallel // cria o time de threads uma vez por repeticao
        {
            long long produto_thr = 1; // acumulador privado por thread 
            #pragma omp for // divide as 10 iteracoes entre as threads
            for (int i = 1; i <= n; i++)
                produto_thr *= i; // cada thread multiplica so na sua copia local, sem disputa

            #pragma omp atomic // unica sincronizacao por thread: funde o parcial no total
            produto *= produto_thr;
        }
    }
    end = omp_get_wtime(); // marca o fim da medicao

    printf("Fatorial de %d: %lld\n", n, produto);
    printf("Tempo decorrido: %lf\n", end - start);
    return 0;
}
