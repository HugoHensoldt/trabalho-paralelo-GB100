# Apresentação — Tarefa 4a: produto escalar e sincronização

## Arquivos

- `exemplo_escalar_critical.c` — soma os produtos parciais em `dot` dentro de `#pragma omp critical`, a cada iteração do laço.
- `exemplo_escalar_atomic.c` — mesma ideia, mas com `#pragma omp atomic` (trava só a operação, não um bloco).
- `exemplo_escalar_local.c` — cada thread acumula em uma variável privada (`aux_dot`) durante o laço, e só entra em `critical` **uma vez**, no final, para somar em `dot`.
- `exemplo_escalar_reduction.c` — usa `#pragma omp for reduction(+:dot)`: o compilador faz automaticamente o que a versão `local` faz manualmente.
- `compilar.sh` — compila as 4 versões com `-fopenmp -O2`.
- `bench.sh` — repete `run.sh` para `OMP_NUM_THREADS` = 1, 2, 4, 8 e imprime os tempos em CSV.

## Como as diretivas impactam o desempenho

- `critical` protegido a cada iteração vira um gargalo: todas as threads disputam o mesmo lock a cada soma.
- `atomic` protege só a instrução aritmética (não um bloco de código) — bem mais leve que `critical`, mas ainda paga sincronização a cada iteração.
- Acumular em variável privada e só sincronizar uma vez no final elimina quase toda a disputa — o custo de sincronização deixa de crescer com o tamanho do vetor.
- `reduction` entrega o mesmo ganho da técnica manual (variável privada + merge único), com código mais simples.

## Resultados (N = 20.000.000)

| Threads | critical | atomic | acumulador local | reduction |
|---------|----------|--------|------------------|-----------|
| 1       | 0.307    | 0.133  | 0.021            | 0.025     |
| 2       | 0.648    | 0.248  | 0.015            | 0.014     |
| 4       | 1.045    | 0.494  | 0.010            | 0.010     |
| 8       | 1.905    | 0.333  | 0.023            | 0.016     |

- `critical` **piora** conforme aumentam as threads — mais threads disputando o mesmo lock a cada iteração.
- `atomic` segue a mesma tendência, mas com overhead bem menor.
- `local` e `reduction` melhoram até 4 threads (uso real dos núcleos físicos) e voltam a piorar levemente em 8 (acima de 4 threads é só hyperthreading, sem núcleo físico extra).
- Ordem de desempenho observada em qualquer número de threads: `critical` (pior) → `atomic` → `local` ≈ `reduction` (melhores).

## Como rodar

```bash
./compilar.sh          # gera as 4 versões (critical/atomic/local/reduction)
./bench.sh 20000000 > resultados.csv   # N=20.000.000, roda 1/2/4/8 threads
```
