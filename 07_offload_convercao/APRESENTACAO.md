# Apresentação — Tarefa 7: de OpenMP Offload para CUDA

Conversão de `off_simples.c` (soma de vetores via `#pragma omp target`) para
CUDA puro. Compilação/execução são feitas no SDumont (sem GPU local) —
este documento traz o código, os scripts e o resultado esperado.

## Arquivos

- `off_simples-original.c` — versão original, OpenMP offload (`#pragma omp target map(...)`).
- `off_simples.cu` — versão convertida para CUDA (kernel `__global__` + API `cuda*`).
- `compilar.sh` — `module load cuda` + `nvcc off_simples.cu -o off_simples`.
- `run.sh` — submissão SLURM (`sbatch run.sh ./off_simples`), 1 GPU.

## Correspondência OpenMP Offload → CUDA

| OpenMP (`off_simples-original.c`) | CUDA (`off_simples.cu`) |
|---|---|
| Vetores `a,b,c` alocados no stack do host | `malloc` no host + `cudaMalloc(d_a,d_b,d_c)` no device |
| `#pragma omp target map(to: a,b) map(from: c)` | `cudaMemcpy(..., HostToDevice)` para `a,b`; `cudaMemcpy(..., DeviceToHost)` para `c` |
| Laço `for` dentro da região `target` (compilador gera o kernel) | Kernel explícito `__global__ void soma(...)`, um thread por elemento (`i = blockIdx.x*blockDim.x + threadIdx.x`) |
| Runtime decide a distribuição de threads no device | Lançamento explícito `soma<<<blocos, threadsPorBloco>>>(...)`, com `blocos = ceil(N/256)` |
| Memória do device liberada implicitamente ao sair do escopo `target` | `cudaFree(d_a,d_b,d_c)` explícito |

A lógica de negócio é idêntica em ambas: `c[i] = a[i] + b[i]`, com
`a[i] = i` e `b[i] = i*2`, logo `c[i] = 3i`.

## Como compilar e rodar no SDumont

```bash
./compilar.sh                 # module load cuda; nvcc off_simples.cu -o off_simples
sbatch run.sh ./off_simples    # submete via SLURM (fila com GPU)
cat slurm-<job_id>.out
```

## Resultado esperado

Como `c[i] = a[i] + b[i] = i + 2i = 3i`, os 10 primeiros valores impressos
devem ser:

```
0 3 6 9 12 15 18 21 24 27
```

Esse resultado é determinístico (não depende de arredondamento nem de
condição de corrida — cada thread escreve em uma posição exclusiva de
`c`), então deve bater exatamente com a saída real do `off_simples` no
SDumont.
