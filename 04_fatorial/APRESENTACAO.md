# Apresentação — Tarefa 4b: produto fatorial e sincronização

## Arquivos

- `fatorial_critical.c` — com `#pragma omp critical` (protege `produto *= i`).  so uma thread multiplica por vez
- `fatorial_local.c` — cada thread multiplica numa variável privada (`produto_thr`) e só sincroniza uma vez, no final, com `#pragma omp atomic` (`produto *= produto_thr`).
- `fatorial_reduction.c` — usa `for reduction(*:produto)`: o compilador cria uma cópia privada de `produto` por thread (iniciada em 1, o neutro da multiplicação) e multiplica tudo automaticamente ao final.
- `bench.sh` — repete `run.sh` para `OMP_NUM_THREADS` = 1, 2, 4, 8 e imprime os tempos em CSV.

Detalhe de implementação: com `n=10` o cálculo é instantâneo demais para medir com `omp_get_wtime` numa única execução, então cada programa repete o cálculo completo **100.000 vezes** internamente e soma o tempo total — sem isso, não daria pra comparar as três versões.

## Como as diretivas impactam o desempenho

- `critical` trava toda vez que uma thread quer multiplicar seu resultado — mais threads competindo pelo mesmo lock.
- Acumular em variável privada e sincronizar só uma vez (`fatorial_local.c`) reduz drasticamente quantas vezes o lock é disputado.
- `reduction` automatiza exatamente essa mesma técnica.
- Diferente do produto escalar, `n!` não pode crescer livremente (estoura `long long` a partir de `n=21`) — por isso o "tamanho do problema" aqui é number de repetições, não `n`.

## Como cada versão executa por dentro (exemplo com n=10, 2 threads)

Multiplicação é associativa e comutativa, então dá pra dividir `1×2×...×10` em
dois pedaços independentes e juntar no final — as três versões fazem isso de
formas diferentes.

**`fatorial_reduction.c`** — cada thread ganha uma cópia privada de `produto`
(iniciada em 1) e calcula seu pedaço sem nenhuma sincronização; o compilador
junta as cópias automaticamente no fim da região paralela:
```
thread 0: produto_local0 = 1×2×3×4×5   = 120
thread 1: produto_local1 = 6×7×8×9×10  = 30240
merge automático (fim da região):  produto = 120 × 30240 = 3628800
```

**`fatorial_local.c`** — mesma ideia, mas o programador faz o merge à mão com
`#pragma omp atomic`, uma vez por thread (não uma vez por iteração):
```
thread 0: produto_thr = 1×2×3×4×5  = 120   →  atomic: produto = 1 × 120   = 120
thread 1: produto_thr = 6×7×8×9×10 = 30240 →  atomic: produto = 120 × 30240 = 3628800
```

**`fatorial_critical.c`** — não existe cópia privada: as duas threads
multiplicam direto na **mesma** variável `produto`, uma de cada vez, com
`#pragma omp critical` protegendo cada multiplicação individual. As 10
iterações se intercalam entre as threads em vez de virar 2 produtos parciais:
```
thread 0: produto *= 1  → produto = 1
thread 1: produto *= 6  → produto = 6
thread 0: produto *= 2  → produto = 12
thread 1: produto *= 7  → produto = 84
        ... (10 entradas no critical, uma por iteração, todas serializadas)
resultado final (mesma ordem matemática, custo bem maior): produto = 3628800
```

O resultado final é sempre `3628800` nas três versões — a diferença de
desempenho vem de **quantas vezes** cada uma precisa travar algo: 1 vez por
thread (`reduction`/`local`) contra 10 vezes no total, uma por iteração
(`critical`).

## Resultados (n = 10, 100.000 repetições, média de 2 execuções)

| Threads | critical | local (thread-private + atomic) | reduction |
|---------|----------|---------------------------------|-----------|
| 1       |    0.034 |              0.033              |   0.022   | 
| 2       |    0.078 |              0.075              |   0.060   |
| 4       |    0.150 |              0.120              |   0.103   |
| 8       |    0.372 |              0.304              |   0.297   |


- local fica perto do critical porque multiplicar não tem instrução atômica nativa (o atomic vira um loop caro de compare-and-swap, quase tão caro quanto um lock), e com só 10 iterações a vantagem de "travar 1x por thread" em vez de "1x por iteração" é pequena demais pra compensar isso.
- A hierarquia se mantém igual à do produto escalar: `critical` sempre o mais lento, `reduction` sempre o mais rápido, em qualquer número de threads.
- A diferença relativa entre versões é bem menor que no produto escalar, porque `n=10` gera poucas disputas de lock por repetição — o custo domina mais é o de criar a região paralela em si.
- **Todas as versões pioram com mais threads** — o oposto do produto escalar. Como o trabalho por região paralela é ínfimo (10 multiplicações), o custo de coordenar mais threads supera qualquer ganho: prova prática de que paralelismo só compensa quando há trabalho suficiente por thread.

## Como rodar

```bash
./compilar.sh          # gera as 3 versões (critical/local/reduction), n=10, 100.000 repetições
./bench.sh > resultados.csv   # roda 1/2/4/8 threads
```
