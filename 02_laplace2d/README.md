# 02 — Laplace 2D: curvas de desempenho

## Objetivo

Três experimentos de desempenho do Jacobi 2D, todos em função do número de
threads OpenMP de 1 até o número de núcleos **físicos** da máquina de teste
(4 no Intel i5-1135G7; ver `README.md` do `Laboratorio_2`):

1. **`laplace2d_gcc.c`** — malha 4096×4096. Curvas para `iter_max=50` e
   `iter_max=500`.
2. **`laplace2d_1k_gcc.c`** — mesma versão, malha reduzida para 1024×1024.
   Curvas para `iter_max=50` e `iter_max=500`.
3. **`laplace2d_1loop_gcc.c`** — os dois `#pragma omp parallel for` de cada
   iteração fundidos em uma única `#pragma omp parallel` (com dois
   `#pragma omp for` dentro). Curva para `iter_max=500`, comparada com
   `laplace2d_gcc.c` (não fundido, mesmo `iter_max`) como baseline.

Resultados, tabelas e discussão de cada experimento: `resultados.md`.

Nos três `.c`, o cálculo de erro (`error = fmax(...)`) aparece **comentado**:
`error` é `shared` e seria atualizada por várias threads sem
`reduction`/`atomic` — uma condição de corrida. O `while` de cada arquivo é
controlado só por `iter_max`.

## Arquivos

- `laplace2d_gcc.c` — sem cálculo de erro (comentado), `NN`/`NM`/`ITER_MAX`
  configuráveis via `-D` (padrão 4096×4096, `ITER_MAX=50`). O binário
  `laplace2d_gcc_i500` também serve de baseline "loops separados" do
  experimento 3.
- `laplace2d_1k_gcc.c` — igual ao anterior, padrão 1024×1024.
- `laplace2d_1loop_gcc.c` — loops fundidos em uma região paralela, cálculo
  de erro comentado, `ITER_MAX` configurável (padrão 500).
- `timer.h` — cópia do original (`jacobi/timer.h`).
- `compilar.sh` — compila todos os binários usados nas curvas (um por
  combinação arquivo × `ITER_MAX`), com `gcc -fopenmp -O2`.
- `bench.sh [max_threads] [repeticoes] [arquivo_saida.csv]` — roda os
  programas listados em `$PROG_LIST` (ou todos, se `PROG_LIST` não for
  definida) com `OMP_NUM_THREADS` de 1 até `max_threads` (padrão 4),
  `repeticoes` vezes cada (padrão 5), grava o CSV e imprime a média por
  configuração.
- `plot.py` — lê `resultados.csv` e gera os 3 gráficos por experimento
  (tempo x threads e speedup x threads, todos com a mesma escala de Y no
  speedup, sem a linha de "speedup ideal"): `curva_exp1_malha4096.png`,
  `curva_exp2_malha1024.png`, `curva_exp3_fusao.png`; mais um gráfico
  combinado com o tempo de todos os experimentos numa escala log:
  `curva_todos_tempos.png`. Requer `matplotlib`
  (`pip3 install --user --break-system-packages matplotlib` no WSL).

## Como rodar

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/02_laplace2d"
./compilar.sh

# grupo rapido: laplace2d_gcc_i50, laplace2d_1k_gcc_i50, laplace2d_1k_gcc_i500
./bench.sh 4 5

# grupos lentos (iter_max=500, malha 4096 -- ~24-32s por execucao):
# rodar em chamadas separadas para nao disputar CPU entre si
PROG_LIST='laplace2d_gcc_i500'        ./bench.sh 4 3 out_exp1_i500.csv
PROG_LIST='laplace2d_1loop_gcc_i500'  ./bench.sh 4 3 out_exp3.csv

python3 plot.py   # gera os 3 PNGs a partir de resultados.csv
```

## Resultados

Ver `resultados.md` — tabelas e resposta a cada pergunta ("a curva de
speedup se altera? por quê?").
