# Apresentação — Tarefa 9: Jacobi 2D com MPI (halo exchange)

## As duas variantes

A diferença é só *quem* copia a coluna de borda para
enviar pela rede — o código C (`buffer`) ou o próprio MPI (`vector`).
Resto do algoritmo é idêntico:

- **`jacobi_mpi_buffer.c`** — empacota a coluna num buffer temporário
  (`double[1024]`) antes do `MPI_Isend`, e desempacota o buffer recebido na
  coluna fantasma depois do `MPI_Waitall`. Duas cópias explícitas por
  iteração, feitas pela aplicação.
- **`jacobi_mpi_vector.c`** — descreve a coluna com `MPI_Type_vector`
  (elementos com passo = largura da linha alocada) e manda o `Isend`/`Irecv`
  direto na matriz local, sem buffer nem cópia manual — a implementação MPI
  cuida de ler/escrever os elementos espaçados.

As trocas de linha (contíguas) são idênticas nas duas versões.

## Resultado (SDumont)

Malha 4096×4096, 16 processos, `ITER_MAX=50`, job 11589465 (`bench.sh`,
10 repetições por variante):

| Variante            | Halo de colunas             | Tempo médio (s) | Desvio padrão (s) |
|---------------------|------------------------------|------------------|--------------------|
| `jacobi_mpi_buffer` | buffer manual (pack/unpack)  | 0.184974         | 0.001374           |
| `jacobi_mpi_vector` | `MPI_Type_vector`            | 0.184646         | 0.003063           |

**Conclusão:** empate dentro do ruído (diferença ~0,3 ms, menor que o
próprio desvio padrão). Faz sentido: o stencil de 5 pontos sobre o bloco
1024×1024 domina o tempo de cada iteração; copiar 1024 doubles (8 KB) para
um buffer é desprezível perto disso, então o `MPI_Type_vector` não rende
ganho mensurável neste tamanho de problema — o benefício dele tende a
aparecer mais quando a comunicação pesa mais no tempo total (trocas
maiores/mais frequentes, ou menos cálculo por iteração).

Dados brutos: `resultados.csv`. Detalhes: `resultados.md`.
