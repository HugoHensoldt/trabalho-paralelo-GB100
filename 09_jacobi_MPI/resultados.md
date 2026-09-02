# Resultados — Tarefa 9 (Jacobi 2D com MPI)

> A preencher após rodar `sbatch bench.sh` no SDumont (ver limitação de
> ambiente em `README.md` — sem toolchain MPI disponível localmente).

## Configuração

- Malha global: 4096×4096, `ITER_MAX=50`.
- 16 processos MPI, topologia cartesiana 4×4 (2 nós × 8 processos/nó).
- 10 repetições por variante (`REPS` em `bench.sh`).

## Tabela

| Variante             | Tempo médio (s) | Desvio padrão (s) |
|----------------------|------------------|--------------------|
| `jacobi_mpi_buffer`  |                  |                    |
| `jacobi_mpi_vector`  |                  |                    |

## Discussão

- A variante com `MPI_Type_vector` elimina o laço de empacotamento/
  desempacotamento das colunas em buffer (executado pela aplicação a cada
  iteração), deixando a cópia por conta da implementação MPI. Comparar se
  essa diferença é mensurável no tempo total, ou se fica dominada pela
  latência de rede entre os 2 nós.
