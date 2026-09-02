# Tarefa 8 — Cálculo de Pi: OpenMP puro vs. MPI puro vs. Híbrido (MPI+OpenMP)

Implementação do cálculo de Pi por integração numérica (regra do ponto médio),
mesma fórmula usada em `MPI_LAB4_2026/teste04.c` e `MPI_LAB4_2026/calc_pi_MPI.c`,
comparando 3 estratégias de paralelização no cluster SDumont:

| Arquivo         | Estratégia                  | Recursos do SDumont                                    |
|-----------------|------------------------------|----------------------------------------------------------|
| `pi_openmp.c`   | OpenMP puro                  | 1 nó, `OMP_NUM_THREADS` = 1..6 (threads nos núcleos)     |
| `pi_mpi.c`      | MPI puro                     | 2 nós, número de processos = 1..6                        |
| `pi_hybrid.c`   | Híbrido MPI + OpenMP         | 2 nós, 2 processos (1 por nó), `OMP_NUM_THREADS` = 1..6 por processo |

`N = 2×10⁹` intervalos em todas as configurações. Cada ponto do gráfico é a
**média de 30 execuções independentes**.

## Algoritmo

Todas as 3 versões calculam a mesma integral (regra do ponto médio):

```
pi = ∫₀¹ 4/(1+x²) dx  ≈  h · Σ 4/(1+xᵢ²),   xᵢ = h·(i − 0.5),   h = 1/N
```

- **`pi_openmp.c`**: um único laço `for (i = 1..N)` com
  `#pragma omp parallel for reduction(+:sum)` — todo o trabalho fica em 1 nó,
  dividido entre as threads.
- **`pi_mpi.c`**: decomposição **entrelaçada** entre ranks — o rank `myid`
  soma os índices `myid+1, myid+1+P, myid+1+2P, ...` (mesmo padrão de
  `teste04.c`/`calc_pi_MPI.c`), e `MPI_Reduce` combina as somas parciais no
  rank 0.
- **`pi_hybrid.c`**: mesma decomposição entrelaçada entre os 2 processos MPI,
  mas a fatia local de cada rank é paralelizada internamente com
  `#pragma omp parallel for reduction(+:sum)` em vez de somada
  sequencialmente — combina os dois níveis de paralelismo.

> **Nota sobre `calc_pi_MPI.c`:** o arquivo de referência em `MPI_LAB4_2026/`
> tem um bug de chaveamento — por falta de um `}` de fechamento, o
> `MPI_Reduce`, a medição final de tempo e o `printf` ficam aninhados dentro
> do `for` e do `if (n != 0)`, então só executam durante a última iteração do
> laço (e nunca, se `n == 0`). `pi_mpi.c` e `pi_hybrid.c` corrigem isso: cada
> rank soma sua fatia inteira e só então participa de um único `MPI_Reduce`.

## Medição de tempo

- `pi_openmp.c`: `omp_get_wtime()` em torno do laço paralelo.
- `pi_mpi.c` / `pi_hybrid.c`: `MPI_Barrier` antes de iniciar, `MPI_Wtime()`
  local em cada rank, e `MPI_Reduce(..., MPI_MAX, ...)` para reportar o maior
  tempo entre os ranks (tempo de parede real da execução distribuída, não a
  média/otimista).

## Compilação (SDumont, toolchain Intel PSXE)

```bash
bash compilar.sh
```

Gera `pi_openmp` (`icc -qopenmp`), `pi_mpi` (`mpiicc`) e `pi_hybrid`
(`mpiicc -qopenmp`), seguindo o mesmo módulo (`intel_psxe/2020_sequana`) e
padrão de `MPI_LAB4_2026/compilar.sh`.

## Execução / benchmark (SDumont, via `sbatch`)

Cada script de benchmark já é o próprio job Slurm: aloca os recursos, varre
as configurações (1 a 6 processos/threads), roda 30 repetições por
configuração e grava um CSV.

```bash
sbatch bench_openmp.sh    # 1 nó,  OMP_NUM_THREADS = 1..6      -> resultados_openmp.csv
sbatch bench_mpi.sh       # 2 nós, processos MPI   = 1..6      -> resultados_mpi.csv
sbatch bench_hybrid.sh    # 2 nós, 2 processos, threads = 1..6 -> resultados_hibrido.csv
```

Acompanhar/gerenciar os jobs como nos demais laboratórios:

```bash
squeue -u $USER
scancel NUMERO_DO_JOB
```

Em `bench_mpi.sh`, o `srun -n $P -m cyclic` distribui os processos
alternando entre os 2 nós alocados, para que mesmo com poucos processos
(ex.: `P=2`) o trabalho fique de fato espalhado pelos 2 nós, e não só
empilhado no primeiro.

## Limitação deste ambiente de desenvolvimento

Esta máquina (Windows + WSL) não tem acesso ao SDumont nem a um toolchain
MPI instalado (sem `sudo` disponível no WSL para instalar `openmpi`), então
**apenas `pi_openmp.c` pôde ser testado localmente** (compilado com
`gcc -fopenmp -O2`, sem o módulo Intel/MPI):

```
threads=1  n=100000000  pi=3.1415926535904264  tempo_s=0.109161
threads=2  n=100000000  pi=3.1415926535899099  tempo_s=0.064204
threads=4  n=100000000  pi=3.1415926535896825  tempo_s=0.040188
threads=8  n=100000000  pi=3.1415926535898158  tempo_s=0.053212
```

Valor de Pi correto em todas as configurações, e tempo cai até 4 threads
(núcleos físicos desta máquina de teste) — confirma que a lógica de
paralelização e a fórmula estão corretas antes de submeter no cluster.
`pi_mpi.c` e `pi_hybrid.c` foram revisados manualmente (decomposição de
índices conferida à mão, ver comentários no código) mas **precisam ser
executados no SDumont** para gerar `resultados_mpi.csv` e
`resultados_hibrido.csv` reais — ver `resultados.md`.

## Arquivos

- `pi_openmp.c`, `pi_mpi.c`, `pi_hybrid.c` — as 3 implementações.
- `compilar.sh` — compila as 3 versões (Intel PSXE, SDumont).
- `bench_openmp.sh`, `bench_mpi.sh`, `bench_hybrid.sh` — jobs Slurm
  (`sbatch`) que rodam a varredura completa (1 a 6) × 30 repetições e geram
  os CSVs.
- `resultados.md` — tabela final e análise (a preencher após rodar no
  SDumont).
