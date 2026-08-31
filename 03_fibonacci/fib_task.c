#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

///////////////////////////////////////////////
//   FIBONACCI RECURSIVO COM OMP TASK
///////////////////////////////////////////////

int fib(int n) {
    int i, j;
    if (n < 2) {
        return n;
    } else {
        // cria uma task assincrona para cada sub-chamada; shared(i)/shared(j)
        #pragma omp task shared(i)
        i = fib(n - 1);
        #pragma omp task shared(j)
        j = fib(n - 2);
        // espera as duas tasks acima (e suas subtasks recursivas) terminarem
        // antes de somar i+j
        #pragma omp taskwait
        return i + j;
    }
}

int main(int argc, char *argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 25;
    int repeticoes = (argc > 2) ? atoi(argv[2]) : 1;
    int res, r;
    double inicio, fim;

    inicio = omp_get_wtime(); // omp_get_wtime: relogio do OpenMP
    
    #pragma omp parallel // cria o time de threads uma unica vez; as tasks de fib() 
    {
        // so uma thread chama fib(); as demais ficam disponiveis no pool para executar as tasks que essa chamada for criando
        #pragma omp single
        // enunciado  chama fib(n) uma unica vez; o laço de repeticoes
        //  foi adicionado so para benchmark, pois era rapida demais para medir com omp_get_wtime
        for (r = 0; r < repeticoes; r++) {
            res = fib(n);
        }
    }
    fim = omp_get_wtime();

    printf("fib(%d) = %d\n", n, res);
    printf("tempo = %lf\n", (fim - inicio) / repeticoes);

    return 0;
}
