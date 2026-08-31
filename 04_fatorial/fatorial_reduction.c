#include <stdio.h>
#include <omp.h>

int main() {
    long long produto;
    int n = 10;
    int reps = 100000;    
    double start, end;

    start = omp_get_wtime();            // marca o inicio da medicao 
    for (int r = 0; r < reps; r++) {    // repete o calculo inteiro 100000x
        produto = 1;
        // reduction(*:produto): cria copia privada de produto por thread, iniciada em 1
        // (neutro da multiplicacao), e multiplica tudo automaticamente ao final 
        #pragma omp parallel for reduction(*:produto)
        for (int i = 1; i <= n; i++) {
            produto *= i;         // O operador '*' inicializa as variaveis locais com 1
        }
    }
    end = omp_get_wtime(); // marca o fim da medicao

    printf("Fatorial de %d: %lld\n", n, produto);
    printf("Tempo decorrido: %lf\n", end - start);
    return 0;
}
