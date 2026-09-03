#include <stdio.h>
#include <omp.h>
#include <stdlib.h> // Para malloc e free

/* off_codigo3.c
 *
 * Demonstra gerenciamento EXPLÍCITO e NÃO ESTRUTURADO de dados no offload
 * para GPU com OpenMP: 
 * "target enter data", 
 * "target update to/from" e
 * "target exit data". 
 * 
 * Ao contrário de um "#pragma omp target map(...)"
 * isolado (que copia os dados a cada região), aqui os vetores são
 * transferidos para o dispositivo UMA vez, permanecem residentes na GPU
 * ao longo de várias regiões "target", e só os elementos que realmente
 * mudam são sincronizados sob demanda. O objetivo é reduzir o tráfego
 * host<->device, que costuma ser o principal gargalo de desempenho em
 * aplicações de GPU.
 *
 * Executado de fato em GPU no Santos Dumont (nvc -mp=gpu, fila
 * sequana_gpu_dev, job SLURM 11589424) — a saída real bateu 100% com os
 * valores "Esperado" cravados nos printf's abaixo. Ver README.md e
 * APRESENTACAO.md nesta pasta para o log completo da execução.
 */

// Função auxiliar para imprimir parte de um array
void print_array_snippet(const char *name, double *arr, int n_total, int snippet_size) {
    printf("%s (primeiros %d elementos): {", name, snippet_size);
    for (int i = 0; i < snippet_size && i < n_total; i++) {
        printf(" %.2f%s", arr[i], (i == snippet_size - 1 || i == n_total - 1) ? "" : ",");
    }
    printf(" }\n");
}

int main() {
    const int N = 20; // Tamanho reduzido para facilitar a visualização
    const int PRINT_SNIPPET = 5;
    double *host_source = (double*)malloc(N * sizeof(double));
    double *host_target = (double*)malloc(N * sizeof(double)); // Para receber resultados e ser modificado

    // 1. Iniciando os dados no Hospedeiro
    for (int i = 0; i < N; i++) {
        host_source[i] = i * 1.0;
        host_target[i] = -1.0; // Valor inicial para ver a mudança
    }
     printf("Dados inciados no hospedeiro.\n");

    // 2. TARGET ENTER DATA (região de dados NÃO estruturada)(forma explicita)
    //     map(to: host_source[0:N])  -> aloca no device e COPIA host->device (entrada apenas)
    //     map(alloc: host_target[0:N]) -> apenas ALOCA no device, sem copiar dado nenhum
    //    Otimização: vetores na GPU durante todo o restante do programa
    #pragma omp target enter data map(to: host_source[0:N]) map(alloc: host_target[0:N])
    printf("Dados mapeados/alocados no dispositivo.\n");

    // 3. PRIMEIRA REGIÃO TARGET: multiplicação no dispositivo 
    //    ao usar host_target e host_source ja estamos com eles na GPU 
    //    mas nao ta paralelo
    #pragma omp target
        for (int i = 0; i < N; i++)
            host_target[i] = host_source[i] * 2.0;  //modificado na GPU
    printf("host_target modificado no dispositivo.\n");

    // 4. TARGET UPDATE FROM: sincroniza device -> host
    //    Copia o conteúdo atual de host_target (calculado na GPU) de volta
    //    para a variável do hospedeiro, SEM desalocar nada no device.
    #pragma omp target update from(host_target[0:N])
    print_array_snippet("Hospedeiro: host_target (após update from)", host_target, N, PRINT_SNIPPET);

    // 5. Modificação local no HOST e TARGET UPDATE TO: sincroniza host -> device
    host_source[0] = 100.0;   // pq esse nao ta sendo modificado na GPU? fora da região
    print_array_snippet("Hospedeiro: host_source (após modificação local)", host_source, N, PRINT_SNIPPET);

    //    atualiza-se APENAS o elemento alterado (host_source[0:1]).
    #pragma omp target update to(host_source[0:1]) // minimiza o moviemnto de dados
    printf("host_source[0] atualizado do hospedeiro para o dispositivo.\n");

    // 6. SEGUNDA REGIÃO TARGET: reaproveita os dados já residentes no device
    #pragma omp target
        for (int i = 0; i < N; i++)
            host_target[i] = host_source[i] + 5.0; // Nova operação
    printf("Segunda computação modificando host_target no dispositivo.\n");

    // 7. TARGET EXIT DATA: fecha o ciclo de vida dos dados no dispositivo
    //    map(from: host_target[0:N]) -> copia device->host (resultado final) e libera o buffer
    //    map(delete: host_source[0:N]) -> apenas libera o buffer no device, sem copiar de volta
    #pragma omp target exit data map(from: host_target[0:N]) map(delete: host_source[0:N])
    printf("Dados finais de host_target transferidos para o hospedeiro, memória liberada no dispositivo.\n");
    
    printf("Verificação final dos dados no Hospedeiro:\n");
    // Deve ser o valor modificado em host_source[0]
    print_array_snippet("Hospedeiro: host_source (final)", host_source, N, PRINT_SNIPPET);
    // Deve refletir a segunda computação
    print_array_snippet("Hospedeiro: host_target (final)", host_target, N, PRINT_SNIPPET); 
    // Verificando alguns valores esperados para host_target
    printf("Verificação de host_target[0]: %.2f (Esperado: 100.0 + 5.0 = 105.0)\n", host_target[0]);
    if (N > 1) 
        printf("Verificação de host_target[1]: %.2f (Esperado: 1.0 + 5.0 = 6.0)\n", host_target[1]);
    // Liberando memória do host
    free(host_source);
    free(host_target);
    return 0;
}