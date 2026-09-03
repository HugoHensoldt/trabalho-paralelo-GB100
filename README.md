# Laboratório 2 — GB100 Processamento Paralelo

## Objetivo

Sequência de 6 trabalhos práticos em OpenMP, cada um explorando uma estratégia
diferente de paralelização e seu impacto em desempenho e corretude:

1. **Escalonamento de laços** (`schedule static/dynamic/guided`) sobre uma matriz.
2. **Laplace 2D** (stencil iterativo) — paralelização de um solver clássico.
3. **Fibonacci** — paralelismo por tarefas (`omp task`) em um problema recursivo.
4. **Produto escalar com sincronização** — comparação de mecanismos de
   sincronização (`critical`, `atomic`, acumulador local, `reduction`) na
   combinação de resultados parciais entre threads.
5. **SIMD** — vetorização explícita (`omp simd`) e seu efeito sobre laços numéricos.
6. **Offload / análise** — consolidação dos resultados anteriores (e, se aplicável,
   diretivas de offload para acelerador) com análise comparativa de desempenho.

Cada pasta é autocontida: código-fonte, script de compilação e, quando fizer
sentido, script de execução com variação do número de threads.

## Estrutura de pastas

```
Laboratorio_2/
├── 01_matriz_schedule/
├── 02_laplace2d/
├── 03_fibonacci/
├── 04_escalar/
├── 05_simd/
├── 06_offload_analise/
└── README.md
```

As pastas `exemplo_*`, `jacobi`, `flush`, `racecar` e o arquivo `MANUAL` já
existiam neste diretório (material de apoio da disciplina, com scripts voltados
para submissão no cluster Sdumont via `sbatch`) e não foram alteradas.

## Ambiente de desenvolvimento

Verificado nesta máquina em 2026-08-21:

- **CPU:** Intel Core i5-1135G7 — **4 núcleos físicos**, 8 threads lógicas
  (hyperthreading 2x). Para benchmarks de escalonamento, o intervalo relevante
  de `OMP_NUM_THREADS` é de 1 a 4 (ganho real); acima disso mede-se o efeito do
  SMT, não paralelismo físico adicional.
- **Compilador:** não há gcc/clang/MSVC no PATH do Windows. O **WSL (Ubuntu)**
  está instalado e tem **gcc 13.3.0 com suporte a `-fopenmp` funcionando**
  (testado e confirmado). Todo o trabalho prático deve ser compilado/executado
  dentro do WSL.

### Como compilar e rodar cada tarefa

Cada pasta terá um `.c` e (quando necessário) um script `compilar.sh`. Padrão
geral, a partir do WSL:

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/<pasta_da_tarefa>"
gcc -fopenmp -O2 -o programa programa.c
OMP_NUM_THREADS=4 ./programa
```

Ou, direto do PowerShell, sem entrar interativamente no WSL:

```powershell
wsl -d Ubuntu -- bash -lc "cd '/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/<pasta_da_tarefa>' && gcc -fopenmp -O2 -o programa programa.c && OMP_NUM_THREADS=4 ./programa"
```

Cada pasta de tarefa incluirá instruções específicas (parâmetros, variações de
`OMP_NUM_THREADS`, comparação de resultados) quando o código for implementado.

## Status

- [x] **01_matriz_schedule** — implementada e com resultados coletados
  (ver `01_matriz_schedule/README.md`).
- [x] **02_laplace2d** — implementada e com resultados coletados
  (ver `02_laplace2d/resultados.md`).
- [ ] 03_fibonacci
- [ ] 04_escalar
- [ ] 05_simd
- [ ] 06_offload_analise
- [ ] 07_offload_convercao
- [ ] **08_pi_MPI** — código pronto (`pi_openmp.c`/`pi_mpi.c`/`pi_hybrid.c` +
  scripts `sbatch`), pendente de execução no SDumont (sem MPI/cluster
  disponível neste ambiente de desenvolvimento; ver `08_pi_MPI/README.md`).
- [x] **09_jacobi_MPI** — implementada e com resultados coletados no
  SDumont (ver `09_jacobi_MPI/resultados.md`).
