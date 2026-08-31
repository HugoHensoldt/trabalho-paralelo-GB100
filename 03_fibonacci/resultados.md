# Resultados — Tarefa 3 (Fibonacci: paralelismo por tarefas)

Máquina: Intel i5-1135G7, 4 núcleos físicos, 8 threads lógicas (hyperthreading).
Tempo em segundos (`omp_get_wtime`), mediana de 3 execuções por configuração.

## O código

`fib_task.c` implementa exatamente o algoritmo do enunciado: uma `fib`
recursiva onde cada chamada gera duas `omp task` (uma para `fib(n-1)`, outra
para `fib(n-2)`) e sincroniza com `taskwait`.

Uma correção em relação ao trecho do enunciado: as variáveis `i` e `j` são
locais ao *stack frame* de cada chamada de `fib`, e não são compartilhadas por
nenhuma construção OpenMP que as englobe — pela regra de *default data-sharing*
do OpenMP, sem uma cláusula explícita elas ficam `firstprivate` dentro da
`task`. Isso faz com que `i = fib(n - 1)` grave numa cópia privada da task, e o
`return i + j` do frame original leia lixo. Por isso, `fib_task.c` usa
`#pragma omp task shared(i)` / `shared(j)`, como no exemplo canônico de
tasking do OpenMP — sem isso o programa roda, mas devolve um resultado errado.

`fib_task_cutoff.c` é o mesmo algoritmo com uma única mudança: abaixo de
`CUTOFF = 20`, resolve `fib` sequencialmente em vez de criar novas tasks
(função auxiliar `fib_seq`, sem `#pragma omp`), eliminando o overhead de
tasks minúsculas.

Mantemos duas curvas de desempenho, porque respondem perguntas diferentes:

1. **F(25)** — o valor pedido atualmente no enunciado.
2. **Varredura de n (28 a 44)** — o comportamento do mesmo código numa faixa
   de tamanhos onde uma única chamada já dura o suficiente para ser medida
   diretamente, sem artifícios de medição.

## 1) F(25) — precisa rodar várias vezes e tirar média

Uma única chamada `fib(25)` dura poucos milissegundos (e frações de
milissegundo na versão com corte), tempo comparável ao ruído do
sistema/agendador — não dá para confiar numa leitura isolada de
`omp_get_wtime`. Por isso os dois programas aceitam um segundo argumento
(`repeticoes`): o cálculo é repetido várias vezes dentro da mesma região
paralela (30x na versão sem corte, 2000x na versão com corte, já que ali cada
chamada é ainda mais rápida) e o tempo reportado é a média.

| Threads | sem corte (s) | com corte (s) |
|---------|---------------|----------------|
| 1       | 0.014986      | 0.000129       |
| 2       | 0.054829      | 0.000089       |
| 4       | 0.091212      | 0.000077       |
| 8       | 0.120775      | 0.000080       |

![F(25) — tempo e speedup x threads](fibonacci_f25.png)

**Resultado:** o código do enunciado (sem corte) piora monotonicamente com
mais threads — de ~15ms com 1 thread para ~121ms com 8 threads (quase 10x mais
lento), porque cada uma das ~242 mil chamadas gera uma task, mesmo as
triviais, e o overhead de criar/agendar cada uma domina sobre o trabalho útil.
A versão com corte já é ~150x mais rápida com 1 thread, e ganha um pouco mais
com threads adicionais (~1.7x em 4 threads) — mas o ganho é modesto porque
`CUTOFF=20` só deixa tasks nos níveis 21-25, pouco trabalho para dividir entre
os núcleos.

## 2) Varredura de n (28 a 44) — o mesmo código numa faixa maior

Aqui uma única chamada já dura de dezenas de milissegundos a segundos, então
a medição é direta (sem repetição interna).

**Sem corte** (task por chamada), n=34:

| Threads | tempo (s) |
|---------|-----------|
| 1       | 1.143     |
| 2       | 3.731     |
| 4       | 6.082     |
| 8       | 8.268     |

**Com corte** (`CUTOFF=20`):

| Threads | n=34 (s) | n=40 (s) | n=44 (s) |
|---------|----------|----------|----------|
| 1       | 0.0090   | 0.1672   | 1.1754   |
| 2       | 0.0063   | 0.1258   | 0.8061   |
| 4       | 0.0049   | 0.1061   | 0.6421   |
| 8       | 0.0090   | 0.1119   | 0.4658   |

![Varredura de n — tempo e speedup x threads](fibonacci_sweep.png)

**Resultado:** o mesmo padrão de overhead aparece (sem corte piora com mais
threads), mas agora, com corte e `n` grande o bastante para gerar tasks em
número suficiente, aparece paralelismo real: speedup de até ~2.5x em 8
threads para n=44. Para `n=34` o ganho já desaparece a partir de 4 threads —
poucas tasks acima do corte para ocupar 8 threads.

## Qual das duas é mais condizente com a realidade?

As duas são medições honestas, mas respondem perguntas diferentes:

- A **varredura (28-44)** é mais representativa de como paralelismo por
  tarefas se comporta *em geral*: os tempos já são grandes o bastante para
  medir sem artifícios, e a versão com corte mostra ganho real e crescente
  (até ~2.5x) à medida que o problema cresce — o comportamento típico que a
  técnica deveria demonstrar.
- **F(25)** é fiel ao que o enunciado pede agora, mas é quase um caso
  degenerado para tasking: o trabalho total (~15ms) mal passa do ruído de
  medição, e mesmo a versão corrigida só ganha ~1.7x, porque não há trabalho
  suficiente para ocupar 4 núcleos. É um resultado real, mas de fronteira —
  mostra que o problema é pequeno demais para compensar paralelismo, não o
  ganho típico da técnica.

Por isso mantemos as duas: F(25) responde a pergunta atual da tarefa; a
varredura contextualiza mostrando o mesmo código numa escala onde tasking
realmente compensa.

## Como reproduzir

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/03_fibonacci"
./compilar.sh

# F(25), com repeticao interna
./bench_f25.sh         > resultados_f25_baseline.csv
./bench_f25_cutoff.sh  > resultados_f25_cutoff.csv

# varredura de n, sem repeticao interna
./bench_sweep.sh         > resultados_sweep_baseline.csv
./bench_sweep_cutoff.sh  > resultados_sweep_cutoff.csv

# graficos (precisa de matplotlib; criamos um venv dedicado)
python3 -m venv ~/.venvs/lab2
~/.venvs/lab2/bin/pip install matplotlib
~/.venvs/lab2/bin/python plot.py   # gera fibonacci_f25.png e fibonacci_sweep.png
```
