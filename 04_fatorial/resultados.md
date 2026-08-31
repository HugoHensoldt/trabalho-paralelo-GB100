# Resultados — Tarefa 4 (Produto fatorial: comparação de sincronização)

Máquina: Intel i5-1135G7, 4 núcleos físicos, 8 threads lógicas (hyperthreading).
`n = 10` (fixo, como no enunciado), `reps = 100.000` repetições do cálculo
completo para tornar o tempo mensurável com `omp_get_wtime`. Tempo em segundos,
média de 2 execuções.

| Threads | critical | local (thread-private + atomic) | reduction |
|---------|----------|---------------------------------|-----------|
| 1       |    0.034 |              0.033              |   0.022   | 
| 2       |    0.078 |              0.075              |   0.060   |
| 4       |    0.150 |              0.120              |   0.103   |
| 8       |    0.372 |              0.304              |   0.297   |

## Interpretação

- **Em qualquer quantidade de threads, `critical` é a mais lenta e `reduction`
  a mais rápida**, na mesma ordem observada no exercício do produto escalar
  (`04_escalar`) — confirma que a hierarquia de custo entre os três mecanismos
  de sincronização é a mesma, independente do problema.
- **Diferença entre as versões é bem menor aqui do que no produto escalar.**
  Com `n=10`, cada repetição só faz 10 multiplicações — a seção crítica é
  disputada poucas vezes por repetição, então o custo do lock em si é pequeno
  perto do custo de criar a região paralela.
- **Todas as versões ficam mais lentas conforme aumenta o número de threads**
  — o oposto do que aconteceu no produto escalar. Isso acontece porque o
  trabalho por região paralela é ínfimo (10 multiplicações); o custo de
  coordenar mais threads (criação, sincronização, distribuição do laço) supera
  de longe qualquer ganho de paralelismo. É a demonstração prática de que
  paralelizar só compensa quando o trabalho por thread é grande o suficiente
  para amortizar esse overhead — o que não é o caso do fatorial de `n=10`.
- Diferente do produto escalar, `n` não pode ser aumentado livremente para dar
  mais trabalho ao laço: `n!` estoura `long long` a partir de `n=21`. Por isso
  o tempo foi ampliado repetindo o cálculo completo (`reps`), não aumentando
  `n`.

## Como reproduzir

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/04_fatorial"
./compilar.sh
./bench.sh
```
