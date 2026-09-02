# Resultados — Tarefa 8 (Pi: OpenMP vs. MPI vs. Híbrido)

**Pendente de execução no SDumont.** Este diretório de desenvolvimento
(Windows + WSL) não tem acesso ao cluster nem a um toolchain MPI local (ver
`README.md`, seção "Limitação deste ambiente"). Preencher esta tabela após
rodar os 3 jobs:

```bash
sbatch bench_openmp.sh
sbatch bench_mpi.sh
sbatch bench_hybrid.sh
```

Cada um grava seu CSV (`resultados_openmp.csv`, `resultados_mpi.csv`,
`resultados_hibrido.csv`) com todas as 30 repetições por configuração; a
média por configuração também é impressa no final de cada job
(`slurm-<id>.out`).

`N = 2.000.000.000` em todas as configurações.

## OpenMP puro (1 nó)

| Threads | Tempo médio (s) |
|---------|------------------|
| 1       |                  |
| 2       |                  |
| 3       |                  |
| 4       |                  |
| 5       |                  |
| 6       |                  |

## MPI puro (2 nós)

| Processos | Tempo médio (s) |
|-----------|------------------|
| 1         |                  |
| 2         |                  |
| 3         |                  |
| 4         |                  |
| 5         |                  |
| 6         |                  |

## Híbrido MPI + OpenMP (2 processos, 2 nós)

| Threads/processo | Tempo médio (s) |
|-------------------|------------------|
| 1                  |                  |
| 2                  |                  |
| 3                  |                  |
| 4                  |                  |
| 5                  |                  |
| 6                  |                  |

## Interpretação

_A preencher após coletar os dados — pontos a cobrir:_

- Speedup de cada abordagem em função do número de processos/threads
  (comparar com o speedup ideal linear).
- OpenMP puro fica limitado aos núcleos de 1 nó só; MPI puro e híbrido usam
  2 nós — comparar o tempo mínimo atingido por cada abordagem, não só a
  inclinação da curva.
- Overhead de comunicação MPI (`MPI_Bcast`, `MPI_Reduce`) vs. overhead de
  sincronização OpenMP (`reduction`) — qual domina em cada configuração.
- Se o híbrido (2 processos × N threads) supera o MPI puro equivalente
  (2N processos) para o mesmo total de "trabalhadores" — evidência do
  benefício de reduzir o número de processos MPI (menos overhead de rede)
  em favor de threads (comunicação por memória compartilhada dentro do nó).

## Como reproduzir

```bash
# No SDumont:
cd 08_pi_MPI
bash compilar.sh
sbatch bench_openmp.sh
sbatch bench_mpi.sh
sbatch bench_hybrid.sh
squeue -u $USER   # acompanhar
```
