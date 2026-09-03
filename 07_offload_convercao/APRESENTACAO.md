# Apresentação — Tarefa 7: de OpenMP Offload para CUDA

Conversão de `off_simples.c` (soma de vetores via `#pragma omp target`) para
CUDA puro. Compilado e executado no SDumont (job GPU via SLURM) — resultado
real confirmado, ver abaixo.

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

## Resultado real (SDumont, job 11589432)

```bash
[hugo.hensoldt@sdumont17 07_offload_convercao]$ bash compilar.sh
*** COMPILACAO COMPLETADA ***
[hugo.hensoldt@sdumont17 07_offload_convercao]$ sbatch run.sh ./off_simples
Submitted batch job 11589432
[hugo.hensoldt@sdumont17 07_offload_convercao]$ cat slurm-11589432.out
0 3 6 9 12 15 18 21 24 27
```

Bate exatamente com o esperado: `c[i] = a[i] + b[i] = i + 2i = 3i`, para
`i = 0..9`. Confirma que o kernel CUDA reproduz a mesma computação da
região `#pragma omp target` original, um thread por elemento (`c` sem
condição de corrida, pois cada thread escreve em posição exclusiva).
