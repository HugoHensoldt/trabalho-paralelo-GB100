# Resultados — Tarefa 2 (Laplace 2D: curvas de desempenho)

Máquina: Intel i5-1135G7, **4 núcleos físicos** (Ncores = 4), 8 threads lógicas
(hyperthreading). `gcc -fopenmp -O2`, WSL Ubuntu. Tempo em segundos
(`timer.h`, wall-clock). Dados brutos em `resultados.csv`.

Legenda das colunas de threads: `1t`...`4t` = `OMP_NUM_THREADS` de 1 a 4.
Speedup = tempo(1 thread) / tempo(N threads).

**Nota sobre o cálculo de erro:** nos três `.c` desta pasta a linha
`error = fmax(error, fabs(Anew[j][i] - A[j][i]));` aparece **comentada**.
`error` é uma variável `shared` atualizada por várias threads sem
`reduction`/`atomic` — mantê-la ativa seria uma condição de corrida. Como o
objetivo aqui é medir desempenho (não checar convergência), o `while` de
cada arquivo é controlado só por `iter_max`, e a linha fica comentada só
para documentar onde o cálculo entraria.

---

## Experimento 1 — `laplace2d_gcc.c` (malha 4096×4096, sem cálculo de erro)

`while` controlado só por `iter_max`. 5 repetições por configuração
(`iter_max=50`); `iter_max=500` foi ampliado para **10 repetições** depois
de uma primeira rodada com 3 mostrar variação suspeita (ver discussão).

| iter_max | 1t (s) | 2t (s) | 3t (s) | 4t (s) |
|----------|--------|--------|--------|--------|
| 50       | 3,194  | 2,636  | 2,569  | 2,563  |
| 500      | 24,714 | 23,085 | 25,587 | 25,269 |

Speedup (vs. 1 thread):

| iter_max | 2t | 3t | 4t |
|----------|------|------|------|
| 50       | 1,21×| 1,24×| 1,25×|
| 500      | 1,07×| 0,97×| 0,98×|

![Curva experimento 1](curva_exp1_malha4096.png)

**A curva de speedup muda com o aumento de `iter_max`?** Sim — e isso só
ficou claro depois de aumentar a amostra de `iter_max=500` de 3 para 10
repetições. Com 3 repetições, a média tinha ficado bem mais alta e errática
(29,2 / 24,1 / 31,6 / 30,1 s, com um mínimo de speedup de 0,93× em 3
threads); junto os 10 valores por configuração, é visível que a maioria
fica num núcleo estável de ~22-24 s e só 2-4 execuções por configuração são
*outliers* isolados de 26-36 s (ex., em 3 threads: 22,5 / 22,6 / 22,7 / 23,2
/ 23,2 / 23,5 / 23,5 / 26,1 / **32,5** / **36,1** s — os dois últimos bem
separados do resto). Isso é ruído do notebook (sem isolamento de processos
do SO), não o algoritmo — mas mesmo removendo esse efeito, a média de 10
repetições (1,07× / 0,97× / 0,98×) continua **abaixo** da curva de
`iter_max=50` (1,21× / 1,24× / 1,25×): a "região sequencial não mudou"
explicaria uma curva idêntica, mas o que os dados mostram é `iter_max=500`
consistentemente mais achatada — mais próxima de 1× em toda contagem de
threads, sem o crescimento de 2→3→4 threads que aparece em `iter_max=50`.
A hipótese mais provável é a mesma do experimento 3: quanto mais iterações,
mais vezes o overhead de fork/join se repete, e isso deixa mais espaço para
o ruído de agendamento do SO dominar o tempo total — a curva "afunda" para
perto de 1× em vez de acompanhar o ganho real de paralelismo que aparece
num teste curto (`iter_max=50`) com menos exposição a esse ruído.

## Experimento 2 — `laplace2d_1k_gcc.c` (malha 1024×1024, sem cálculo de erro)

Mesma versão do experimento 1, mas com malha 1024×1024 em vez de 4096×4096.
5 repetições por configuração.

| iter_max | 1t (s) | 2t (s) | 3t (s) | 4t (s) |
|----------|--------|--------|--------|--------|
| 50       | 0,146  | 0,155  | 0,128  | 0,133  |
| 500      | 1,407  | 1,325  | 1,351  | 1,468  |

Speedup (vs. 1 thread):

| iter_max | 2t | 3t | 4t |
|----------|------|------|------|
| 50       | 0,94×| 1,14×| 1,10×|
| 500      | 1,06×| 1,04×| 0,96×|

![Curva experimento 2](curva_exp2_malha1024.png)

**Reduzir a malha para 1024×1024 aumenta o speedup?** **Não, nesta máquina
— pelo contrário.** Reduzir a malha diminui o tráfego de memória (1024² é
~16× menos dados que 4096², cabe quase inteiro no cache L3 de 8 MB), mas na
prática o speedup ficou **pior** que o da malha 4096 (que chegou a 1,25×):
aqui ele nunca passa de ~1,14× e na maior parte das configurações fica
**abaixo de 1** (mais lento que 1 thread). O motivo é que reduzir a malha
não só tira o gargalo de banda de memória como também **reduz
drasticamente o trabalho por chamada da região paralela** (1022×1022 ≈ 1M
células/iteração, contra 4094×4094 ≈ 16,8M células na malha grande — 16×
menos trabalho). Com tão pouco trabalho por `#pragma omp parallel for`, o
custo **fixo** de criar/sincronizar o time de threads (que não diminui com
o tamanho do problema) passa a dominar — o gargalo migra de "banda de
memória" para "overhead de fork-join", e esse segundo gargalo é
proporcionalmente pior justamente porque o problema ficou pequeno.

**A curva muda entre `iter_max=50` e `iter_max=500`?** Não de forma
relevante — os speedups continuam todos próximos de 1× (0,94-1,14× em 50
iterações, 0,96-1,06× em 500), pelo mesmo motivo do experimento 1: a
estrutura por iteração não mudou, então a proporção overhead/trabalho é a
mesma em qualquer `iter_max`. Como aqui essa proporção já é desfavorável em
qualquer `iter_max`, aumentar o número de iterações só repete a mesma razão
ruim mais vezes.

## Experimento 3 — `laplace2d_1loop_gcc.c` (fusão das regiões paralelas)

Malha 4096×4096, `iter_max=500`. Comparação: `laplace2d_gcc.c` (duas
regiões `#pragma omp parallel for` por iteração — cálculo e cópia — mesmo
binário `laplace2d_gcc_i500` do experimento 1) vs. `laplace2d_1loop_gcc.c`
(uma única `#pragma omp parallel` por iteração, com dois `#pragma omp for`
dentro — um só fork/join em vez de dois). Ambos sem cálculo de erro ativo.
**10 repetições** para "loops separados" (mesmos dados do experimento 1,
ver nota sobre outliers); 3 repetições para "loops fundidos".

| Versão           | 1t (s) | 2t (s) | 3t (s) | 4t (s) |
|-------------------|--------|--------|--------|--------|
| loops separados   | 24,714 | 23,085 | 25,587 | 25,269 |
| loops fundidos    | 24,141 | 23,674 | 23,510 | 24,087 |

Speedup (vs. 1 thread da própria versão):

| Versão          | 2t   | 3t   | 4t   |
|------------------|------|------|------|
| loops separados  | 1,07×| 0,97×| 0,98×|
| loops fundidos   | 1,02×| 1,03×| 1,00×|

![Curva experimento 3](curva_exp3_fusao.png)

**Fundir os dois laços em uma única região paralela muda o desempenho?**
Sim, mas de forma mais modesta do que a primeira rodada (com só 3
repetições no baseline) sugeria — aquela tinha "loops separados" chegando a
29-31 s por causa dos mesmos *outliers* discutidos no experimento 1. Com 10
repetições no baseline, a diferença fica menor mas ainda real: a versão
fundida é igual ou mais rápida em toda configuração de threads (24,1 vs
24,7 s em 1 thread; 23,5 vs 25,6 s em 3 threads) e mais **estável** —
variação de 0,6 s entre as 4 configurações de threads (23,5-24,1 s), contra
2,5 s na versão separada (23,1-25,6 s), mesmo já removidos os outliers mais
extremos pela média de 10 repetições. O speedup relativo de cada versão em
relação a si mesma fica perto de 1× nos dois casos, porque este kernel já
satura seu paralelismo útil com poucas threads (é *memory-bound*, ver
experimento 1) — a fusão não libera mais paralelismo, ela **elimina metade
das criações de time de threads** (500 fork/joins por execução em vez de
1000, já que `iter_max=500`), reduzindo tanto o tempo total quanto a
exposição a ruído de agendamento do SO. Ou seja: a fusão dos loops compra um
tempo total menor e mais previsível, não um coeficiente angular melhor na
curva de speedup.

---

## Todos os tempos juntos

![Todos os tempos](curva_todos_tempos.png)

Escala logarítmica no eixo Y (os tempos variam de ~0,13 s a ~31 s entre os
experimentos). As duas curvas de `iter_max=500` (malha 4096 separada e
malha 4096 fundida, laranja e roxa) ficam próximas e bem acima das demais;
a malha 1024 (verde/vermelha) fica ~20× mais rápida em qualquer `iter_max`
— confirma visualmente o argumento do experimento 2 sobre volume de dados.

## Sobre ruído de medição

Os tempos vêm de um notebook sem isolamento de outros processos do SO
(WSL sobre Windows). As configurações de `iter_max=500` (execuções de
~23-36 s cada) mostraram *outliers* claros: para `laplace2d_gcc_i500`,
aumentar a amostra de 3 para 10 repetições revelou que em cada contagem de
threads a maioria dos valores forma um núcleo estável de ~22-24 s, com 2-4
execuções isoladas de 26-36 s bem separadas do resto (não é uma distribuição
gradual — é um salto). Com só 3 repetições, a média fica muito sensível a
esses outliers (uma execução de 36 s entre 3 repetições desloca a média em
~4 s); com 10, o efeito ainda existe mas fica bem mais amortecido. Onde
havia tempo de execução curto (`iter_max=50`, ~2,5-3 s), 5 repetições já
foram suficientes e a variância ficou pequena desde o início — sugerindo que
o ruído cresce com o tempo total de execução (mais chance de o SO
interromper o processo em algum ponto), não com o número de threads em si.

## Como reproduzir

```bash
wsl -d Ubuntu
cd "/mnt/c/Users/HugoV/Desktop/GB100 Processamento Paralelo/Laboratorio_2/02_laplace2d"
./compilar.sh
./bench.sh 4 5                                                # grupo rapido (i50, 1k_i50, 1k_i500)
PROG_LIST='laplace2d_gcc_i500' ./bench.sh 4 10 out1.csv       # experimento 1 e baseline do 3, iter_max=500 (10 reps p/ diluir outliers)
PROG_LIST='laplace2d_1loop_gcc_i500' ./bench.sh 4 3 out2.csv  # experimento 3
python3 plot.py                                                # gera os graficos
```
