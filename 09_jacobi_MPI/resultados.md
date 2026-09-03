# Resultados — Tarefa 9 (Jacobi 2D com MPI)

Executado com `sbatch bench.sh` no SDumont. Dados brutos em `resultados.csv`.

## Configuração

- Malha global: 4096×4096, `ITER_MAX=50`.
- 16 processos MPI, topologia cartesiana 4×4 (2 nós × 8 processos/nó).
- 10 repetições por variante (`REPS` em `bench.sh`).

## Tabela

| Variante             | Tempo médio (s) | Desvio padrão (s) |
|----------------------|------------------|--------------------|
| `jacobi_mpi_buffer`  | 0.184974         | 0.001374           |
| `jacobi_mpi_vector`  | 0.184646         | 0.003063           |

## Discussão

- As duas variantes ficam estatisticamente empatadas: 0.184974 s
  (`buffer`) x 0.184646 s (`vector`), uma diferença de ~0.3 ms — bem menor
  que o próprio desvio padrão das 10 repetições de cada uma. Ou seja, para
  esta malha/`ITER_MAX`, trocar o empacotamento manual das colunas por
  `MPI_Type_vector` não trouxe ganho de desempenho mensurável.
- Faz sentido: a coluna de borda trocada por iteração tem 1024 doubles
  (8 KB) — o laço de empacotamento/desempacotamento em `jacobi_mpi_buffer.c`
  é uma cópia sequencial de poucos microssegundos, desprezível perto do
  tempo de comunicação em si (latência de rede entre os 2 nós) e,
  principalmente, perto do custo do stencil de 5 pontos sobre o bloco
  1024×1024 completo a cada uma das 50 iterações — que domina o tempo total
  e é idêntico nas duas variantes. O benefício do `MPI_Type_vector` tende a
  aparecer mais em cenários com muitas trocas pequenas e pouco cálculo por
  iteração (comunicação como fração maior do tempo total), não neste ponto
  de operação.
- O desvio padrão da variante `vector` (0.0031 s) é maior que o da `buffer`
  (0.0014 s) só por causa de 1 execução (`rep=5`, 0.176297 s) destacada das
  demais — ruído do cluster compartilhado (rede/OS jitter entre os 2 nós),
  não uma diferença sistemática entre as implementações. Sem esse outlier,
  as 9 execuções restantes da `vector` ficam na mesma faixa da `buffer`
  (~0.184-0.187 s).
