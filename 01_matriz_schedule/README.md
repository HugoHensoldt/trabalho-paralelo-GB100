# 01 — Multiplicação de matrizes: comparação de `schedule`

## Objetivo

Comparar o tempo de execução da multiplicação de duas matrizes quadradas
`C = A x B`, com `N = 4096`, paralelizada no laço externo com
`#pragma omp for`, para as cláusulas de escalonamento `static`, `dynamic` e
`guided`, cada uma com `chunk = 2` e `chunk = 100`.

`OMP_NUM_THREADS` é fixado em **4**, o número de núcleos físicos da máquina
de teste (Intel i5-1135G7 — 4 núcleos físicos / 8 threads lógicas via
hyperthreading; ver README.md do `Laboratorio_2`). Threads lógicas extras via
SMT não são núcleos de execução adicionais, então o experimento mede o
paralelismo físico real.

## Arquivos

- `mult_matriz.c` — multiplicação de matrizes N×N (N=4096 por padrão,
  compilável com outro N via `-DN=...` para calibração). O laço `i` (linhas
  de C) é paralelizado com `schedule(runtime)`, isto é, o par
  `(modo, chunk)` é lido de `OMP_SCHEDULE` em tempo de execução — não é
  necessário recompilar para trocar de cenário (mesmo padrão do
  `exemplo_04_3.c` da pasta `exemplo_4/`).
- `compilar.sh` — compila com `gcc -fopenmp -O2`.
- `run_benchmark.sh [num_threads] [repeticoes]` — roda as 6 combinações
  (`static|dynamic|guided` × `chunk=2|100`), por padrão com
  `OMP_NUM_THREADS=4` e 3 repetições por combinação, grava `resultados.csv`
  e imprime a média por configuração.

## Detalhe de implementação importante: ordem dos laços

O laço usa a ordem **i-k-j** (não i-j-k):

```c
for (i ...)
  for (k ...)
    for (j ...)
      C[i][j] += A[i][k] * B[k][j];
```

Nessa ordem, o laço mais interno acessa `B[k][j]` e `C[i][j]` com passo 1
(linha contígua em memória), o que favorece cache e permite
auto-vetorização pelo compilador. Com a ordem ingênua i-j-k, o acesso a
`B[k][j]` no laço interno teria passo `N=4096` (32 KB) — para matrizes de
128 MB isso gera *cache miss* em praticamente todo acesso, e o tempo medido
passaria a refletir o custo de memória, não o efeito da cláusula de
escalonamento (que é o que a tarefa pede para comparar). A troca de ordem
não muda o número de operações (ainda O(N³)), só a localidade de acesso — é
uma otimização ortogonal à pergunta do experimento.

## Como rodar

No WSL (Ubuntu), a partir desta pasta:

```bash
./compilar.sh
./run_benchmark.sh          # 4 threads, 3 repetições (padrão)
./run_benchmark.sh 4 5      # 4 threads, 5 repetições
```

Ou direto do PowerShell:

```powershell
wsl -d Ubuntu -- bash -lc "cd '/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/01_matriz_schedule' && ./compilar.sh && ./run_benchmark.sh"
```

Para rodar um cenário manualmente (equivalente ao padrão
`exemplo_04_3` / `time ./exemplo_04_3` do material de apoio):

```bash
export OMP_NUM_THREADS=4
export OMP_SCHEDULE="dynamic,2"
time ./mult_matriz
```

## Viabilidade (calibração)

Antes de rodar a bateria completa, o tempo foi calibrado com N menor:

| N    | tempo (4 threads, static) |
|------|----------------------------|
| 1024 | 0,18 s                     |
| 2048 | 2,27 s                     |
| 4096 | 30,3 s                     |

Uma execução em N=4096 leva ~30 s com 4 threads; a bateria completa (6
combinações × 3 repetições = 18 execuções) leva ~9 minutos. É viável rodar
sem reduzir N nem o número de repetições.

## Resultados

Execução real em 2026-08-28, `OMP_NUM_THREADS=4`, 3 repetições por
configuração (dados brutos em `resultados.csv`):

| schedule | chunk | rep 1 (s) | rep 2 (s) | rep 3 (s) | média (s) |
|----------|-------|-----------|-----------|-----------|-----------|
| static   | 2     | 28,50     | 22,80     | 25,41     | **25,57** |
| static   | 100   | 28,20     | 23,40     | 24,43     | **25,35** |
| dynamic  | 2     | 26,19     | 34,86     | 37,42     | **32,82** |
| dynamic  | 100   | 29,83     | 29,03     | 24,30     | **27,72** |
| guided   | 2     | 24,29     | 22,60     | 22,79     | **23,23** |
| guided   | 100   | 27,70     | 25,43     | 23,70     | **25,61** |

Ranking por média (mais rápido → mais lento): `guided,2` (23,23s) <
`static,100` (25,35s) ≈ `static,2` (25,57s) ≈ `guided,100` (25,61s) <
`dynamic,100` (27,72s) < `dynamic,2` (32,82s).

## Discussão

Cada iteração do laço `i` executa ~N² = 16M operações de multiplicação-soma
(uma linha inteira de C), ou seja, o trabalho por iteração é grande e
uniforme entre linhas (a multiplicação de matrizes não tem desbalanceamento
de carga entre iterações, ao contrário de laços com trabalho
dependente do índice). Nessa condição, o resultado observado confirma a
expectativa teórica:

- **`static` foi o mais estável** (25,35–25,57 s, baixa variância entre
  repetições): como a carga já é balanceada, dividir o laço em blocos fixos
  sem coordenação em tempo de execução é suficiente e tem overhead mínimo.
  `chunk=2` e `chunk=100` deram tempos praticamente iguais, porque o
  particionamento round-robin de `static` não depende de sincronização por
  chunk (é decidido antes do laço começar).
- **`dynamic,2` foi claramente o pior** (32,82 s de média, e a maior
  variância entre repetições: 26,2–37,4 s). Com chunk=2, o laço gera 2048
  chunks; cada um exige uma seção crítica implícita (fetch-and-add) para o
  próximo bloco de trabalho, e o load balancing dinâmico não traz benefício
  aqui porque não há desbalanceamento a corrigir — só overhead de
  sincronização. `dynamic,100` (41 chunks) já reduz esse custo (27,72 s),
  confirmando que o problema é a granularidade fina, não o modo `dynamic`
  em si.
- **`guided,2` teve o melhor tempo médio** (23,23 s). `guided` começa com
  chunks grandes e vai reduzindo o tamanho a cada despacho, então o valor de
  `chunk=2` funciona como *tamanho mínimo*, não como tamanho de todo chunk
  — a maior parte do trabalho ainda é despachada em blocos grandes no início,
  evitando o overhead de sincronização que penalizou `dynamic,2`.

Conclusão prática: para uma carga balanceada como multiplicação de
matrizes, `static` é a escolha mais previsível e já é ótima; `dynamic` só
compensa seu overhead de sincronização quando há desbalanceamento real de
carga entre iterações, e é sensível à escolha do chunk (chunk pequeno é
armadilha de desempenho); `guided` tende a ser um meio-termo seguro mesmo
com chunk pequeno, por causa do decaimento automático do tamanho do bloco.

Observação sobre ruído de medição: os tempos vieram de uma única máquina
local (notebook) sem isolamento de outros processos do SO, então variações
de ±3–5 s entre repetições da mesma configuração (visíveis em `dynamic,2`,
por exemplo) refletem também jitter do ambiente, não só o algoritmo — daí a
importância de rodar múltiplas repetições e comparar médias.
