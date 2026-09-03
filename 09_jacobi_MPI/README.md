# Tarefa 9 — Jacobi 2D com MPI: decomposição de domínio e halo exchange

Paralelização por **decomposição de domínio** da iteração de Jacobi 2D usada
em [`02_laplace2d`](../02_laplace2d/laplace2d_gcc.c) (mesma equação — stencil
de 5 pontos — e mesma condição de contorno: coluna global 0 = 1.0, demais
bordas = 0.0), agora distribuída entre **16 processos MPI** organizados numa
**topologia cartesiana 4×4**, com base em
[`MPI_LAB4_2026/teste11.c`](../MPI_LAB4_2026/teste11.c).

A malha global 4096×4096 é dividida em 16 blocos de submatrizes locais
1024×1024 (um por processo). Cada bloco recebe uma borda de **células
fantasma** (ghost cells) de 1 célula de espessura nos 4 lados, preenchida a
cada iteração por uma fase de **halo exchange** com os vizinhos diretos
(cima/baixo/esquerda/direita), usando comunicação não bloqueante
(`MPI_Isend`/`MPI_Irecv`).

## Arquivos

- [`jacobi_mpi_buffer.c`](jacobi_mpi_buffer.c) — linhas de borda são
  contíguas na memória (layout row-major) e enviadas diretamente; colunas de
  borda são empacotadas num buffer temporário antes do envio e
  desempacotadas na coluna fantasma correspondente ao receber.
- [`jacobi_mpi_vector.c`](jacobi_mpi_vector.c) — mesma lógica, mas troca o
  empacotamento manual das colunas por um tipo derivado `MPI_Type_vector`
  (elementos com passo igual à largura da linha alocada), como em
  [`MPI_LAB4_2026/teste07.c`](../MPI_LAB4_2026/teste07.c): o MPI lê/escreve
  a coluna diretamente na matriz local, sem cópia intermediária definida
  pela aplicação.
- `compilar.sh` — compila as 2 variantes (Intel PSXE, SDumont).
- `sub.sh EXECUTAVEL` — job Slurm avulso (16 processos, grid 4×4), igual ao
  padrão de `MPI_LAB4_2026/sub_teste11.sh`.
- `bench.sh` — job Slurm que roda as 2 variantes REPS=10 vezes cada e grava
  `resultados.csv` com o tempo médio de cada uma.
- `resultados.md` — tabela e discussão (a preencher após rodar no SDumont).

## Topologia e decomposição

- `MPI_Cart_create(MPI_COMM_WORLD, 2, dims={4,4}, periods={0,0}, reorder=0, &cartcomm)`
  — mesma chamada de `teste11.c`. `periods={0,0}` faz processos nas bordas
  do grid cartesiano terem vizinho `MPI_PROC_NULL` naquela direção (sem
  wrap-around).
- `coords[0]` é a linha do processo no grid 4×4 (vizinhos cima/baixo,
  `MPI_Cart_shift` na dimensão 0); `coords[1]` é a coluna (vizinhos
  esquerda/direita, dimensão 1) — mesma convenção de `teste11.c`.
- Bloco local: `1024×1024` reais + 1 célula fantasma em cada lado
  (`1026×1026` alocado).
- A condição de contorno de Dirichlet (coluna global 0 = 1.0) só é aplicada
  pelos processos com `coords[1] == 0`; as demais bordas globais (linha 0,
  linha 4095, coluna 4095) ficam em 0.0 (valor inicial). Essas bordas nunca
  são recalculadas pelo stencil — cada processo ajusta sua faixa de
  atualização (`jlo/jhi/ilo/ihi`) para pular a borda global quando está numa
  ponta do grid cartesiano (`coords[*] == 0` ou `coords[*] == 3`).

## Halo exchange (início de cada iteração)

Para cada uma das 4 direções, um `MPI_Isend` (envia a borda real do
processo) é emparelhado com um `MPI_Irecv` do vizinho oposto na mesma
dimensão, usando 4 tags distintas (`TAG_TO_UP/DOWN/LEFT/RIGHT`, uma por
"sentido de viagem" da mensagem) para casar corretamente os 8 envios/
recebimentos não bloqueantes — mesmo padrão de `reqs[8]`/`MPI_Waitall` de
`teste11.c`. Processos na borda do domínio global têm o vizinho
correspondente igual a `MPI_PROC_NULL`; `MPI_Isend`/`MPI_Irecv` para/de
`MPI_PROC_NULL` completam imediatamente sem efeito, então a célula fantasma
daquele lado simplesmente não é atualizada (e nunca é lida, pois cai fora da
faixa de atualização do stencil).

- **Linhas** (`jacobi_mpi_buffer.c` e `jacobi_mpi_vector.c`, idêntico nos
  dois): `&A[1][1]`/`&A[rl][1]` com `count = cl` — contíguas por linha, sem
  necessidade de tipo derivado.
- **Colunas**:
  - `jacobi_mpi_buffer.c`: laço `for j in 1..rl` copia `A[j][1]`/`A[j][cl]`
    para um buffer contíguo antes do `MPI_Isend`; após o `MPI_Waitall`, outro
    laço copia o buffer recebido de volta para a coluna fantasma
    (`A[j][0]`/`A[j][cl+1]`).
  - `jacobi_mpi_vector.c`: `MPI_Type_vector(rl, 1, col_alloc, MPI_DOUBLE, &coltype)`
    descreve a coluna diretamente (passo = largura da linha alocada);
    `MPI_Isend`/`MPI_Irecv` usam esse tipo com `count=1` direto em
    `&A[1][1]`/`&A[1][cl]`/`&A[1][0]`/`&A[1][cl+1]`, sem laço de
    empacotamento na aplicação.

## Medição de tempo

`MPI_Barrier` antes de iniciar as `ITER_MAX` iterações, `MPI_Wtime()` local
em cada rank, e `MPI_Reduce(..., MPI_MAX, ...)` para reportar o maior tempo
entre os 16 ranks (tempo de parede real da execução distribuída) — mesmo
padrão de [`08_pi_MPI/pi_mpi.c`](../08_pi_MPI/pi_mpi.c). Rank 0 imprime o
total.

## Compilação (SDumont, toolchain Intel PSXE)

```bash
bash compilar.sh
```

Gera `jacobi_mpi_buffer` e `jacobi_mpi_vector` (`mpiicc -O2`), seguindo o
módulo `intel_psxe/2020_sequana` de `MPI_LAB4_2026/compilar.sh`.

`N`, `M` (tamanho da malha global) e `ITER_MAX` têm padrão 4096×4096 e 50
iterações (mesmo padrão de `02_laplace2d`), configuráveis via `-D`, ex.:
`mpiicc -O2 -DITER_MAX=500 jacobi_mpi_vector.c -o jacobi_mpi_vector_i500`.
`N`/`M` precisam continuar divisíveis por 4 (`PY`/`PX`), já que o código não
trata resto de divisão no tamanho do bloco local.

## Execução (SDumont, via `sbatch`)

```bash
sbatch sub.sh ./jacobi_mpi_buffer
sbatch sub.sh ./jacobi_mpi_vector

# ou a comparacao completa (10 repeticoes de cada variante):
sbatch bench.sh
```

```bash
squeue -u $USER
scancel NUMERO_DO_JOB
```

## Limitação deste ambiente de desenvolvimento

Como em [`08_pi_MPI`](../08_pi_MPI/README.md#limitação-deste-ambiente-de-desenvolvimento),
esta máquina (Windows + WSL Ubuntu) não tem toolchain MPI instalado e não há
`sudo` disponível no WSL para instalar `openmpi` (`sudo -n true` pede senha).
Não foi possível compilar/executar `jacobi_mpi_buffer.c`/`jacobi_mpi_vector.c`
localmente antes de submeter ao cluster. Antes de rodar no SDumont, a lógica
foi revisada manualmente (topologia cartesiana e `Cart_shift` conferidos
contra `teste11.c`, tipo derivado conferido contra `teste07.c`, casamento de
tags dos 8 `Isend`/`Irecv` por iteração conferido à mão) e validada por uma
simulação em Python do mesmo algoritmo de decomposição/halo exchange contra
o resultado serial de referência (`02_laplace2d`), batendo bit a bit.

## Resultados

`sbatch bench.sh` já foi executado no SDumont (16 processos, grid 4×4,
malha 4096×4096, `ITER_MAX=50`, 10 repetições por variante) — ver tabela e
discussão em [`resultados.md`](resultados.md) e dados brutos em
[`resultados.csv`](resultados.csv). Resumo: as duas variantes empatam
dentro do ruído (~0.185 s em média); a troca do empacotamento manual das
colunas por `MPI_Type_vector` não trouxe ganho mensurável para este
tamanho de problema, pois o custo do stencil por iteração domina o tempo
total muito mais do que a cópia de uma coluna de 1024 doubles.
