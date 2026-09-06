# Resultados — Tarefa 4 (Produto escalar: comparação de sincronização)

Máquina: Intel i5-1135G7, 4 núcleos físicos, 8 threads lógicas (hyperthreading).
`N = 20.000.000` elementos por vetor. Tempo em segundos (`omp_get_wtime`), média
de 3 execuções por configuração (dados brutos variam bastante entre execuções
individuais nas versões `local`/`reduction`, por serem tempos muito curtos).

![Produto escalar — tempo e speedup x threads](grafico_escalar.png)

| Threads | critical | atomic | acumulador local | reduction |
|---------|----------|--------|------------------|-----------|
| 1       | 0.328    | 0.148  | 0.020            | 0.020     |
| 2       | 0.675    | 0.322  | 0.011            | 0.012     |
| 4       | 1.621    | 0.713  | 0.010            | 0.011     |
| 8       | 3.097    | 0.828  | 0.028            | 0.023     |

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
./bench.sh 20000000 > resultados.csv   # rodar 3x e tirar media reduz o ruido
python3 plot.py                          # gera grafico_escalar.png
```
