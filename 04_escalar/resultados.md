# Resultados — Tarefa 4 (Produto escalar: comparação de sincronização)

Máquina: Intel i5-1135G7, 4 núcleos físicos, 8 threads lógicas (hyperthreading).
`N = 20.000.000` elementos por vetor. Tempo em segundos (`omp_get_wtime`).

| Threads | critical | atomic | acumulador local | reduction |
|---------|----------|--------|------------------|-----------|
| 1       | 0.307    | 0.133  | 0.021            | 0.025     |
| 2       | 0.648    | 0.248  | 0.015            | 0.014     |
| 4       | 1.045    | 0.494  | 0.010            | 0.010     |
| 8       | 1.905    | 0.333  | 0.023            | 0.016     |

## Interpretação

- **`critical`**: piora monotonicamente com mais threads. A seção crítica protege
  a soma a cada iteração do laço, então o lock vira o gargalo — mais threads
  disputando o mesmo lock significa mais tempo bloqueado, não menos trabalho.

- **`atomic`**: mesma tendência de piora, mas overhead bem menor que `critical`
  por travar só a instrução aritmética, não um bloco de código.
  
- **acumulador local + `critical` único** e **`reduction`**: desempenho
  equivalente entre si (a `reduction` é essencialmente essa mesma técnica feita
  automaticamente pelo compilador). Ambas melhoram até 4 threads — uso real dos
  4 núcleos físicos — e voltam a piorar levemente em 8, porque acima de 4
  threads lógicas o ganho vem só de hyperthreading disputando as mesmas
  unidades de execução, sem núcleo físico adicional.

## Como reproduzir

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/04_escalar"
./compilar.sh
./bench.sh 20000000
```
