# Apresentação — Tarefa 5: vetorização com `#pragma omp simd`

## Arquivos

- `exemplo_simd_on.c` — laço em faixas (*strip-mining*): `#pragma omp parallel for` divide o vetor entre threads (128 elementos por bloco); dentro de cada bloco, `#pragma omp simd` vetoriza o laço interno. Cada elemento passa por `DoSomeWork` (50 iterações de `x = x*0.99 + 1.0`), pra dar trabalho de CPU suficiente pro SIMD ter efeito.
- `exemplo_simd_off.c` — mesma estrutura de faixas e mesmo `DoSomeWork`, mas **sem** `#pragma omp simd` (só paralelismo de threads).
- `compilar.sh` — compila as 2 versões com `-fno-tree-vectorize -fno-tree-slp-vectorize`, pra impedir o `gcc -O2` de vetorizar sozinho e assim isolar o efeito da diretiva.
- `run.sh` / `bench.sh` — rodam as 2 versões para vários tamanhos de vetor (`N`).

## Como a diretiva impacta o desempenho

- `#pragma omp simd` informa ao compilador que as iterações do laço interno são independentes, liberando a geração de instruções vetoriais (SSE, 128 bits = 2 `double` por instrução nesta CPU).
- O ganho teórico máximo é o tamanho do vetor SIMD do hardware — aqui, 2×.
- A diretiva não tem custo próprio: só ajuda quando o laço já é vetorizável (sem dependência entre iterações) e tem trabalho de CPU suficiente por elemento para não ficar limitado pela banda de memória.

## Resultados (4 threads, média de 3 execuções)

|       N      | com simd | sem simd | speedup |
|--------------|----------|----------|---------|
| 1.000.000    | 0,0087 s | 0,0167 s |  1,91×  |
| 10.000.000   | 0,0745 s | 0,1595 s |  2,14×  |
| 100.000.000  | 0,7440 s | 1,4404 s |  1,94×  |

- Em **todos** os tamanhos testados, `com_simd` foi mais rápido — nunca houve prejuízo por usar a diretiva.
- O speedup fica estável perto de **2×**, batendo exatamente com o hardware (2 `double`/instrução em SSE).
- O ganho poderia sumir (ou até inverter) em laços com dependência entre iterações, laços muito curtos, ou laços limitados por banda de memória em vez de poder de processamento.

## Como rodar

```bash
./compilar.sh          # gera exemplo_simd_on e exemplo_simd_off (sem vetorização automática do gcc)
./bench.sh 4 > resultados.csv   # OMP_NUM_THREADS=4, varre N de 100.000 a 100.000.000
./run.sh 10000000       # roda as 2 versões para um único N, direto no terminal
```
