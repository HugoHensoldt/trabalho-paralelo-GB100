# Apresentação — Tarefa 3: Fibonacci com `omp task` (F=25)


ANDRE DEVIA ESTAR RODANDO COM MUITO CORES ENTAO NO MEU CASO DEU PRATICAMENTE A MSM COISA COM O f=25 
GRAFICO TA LEGAL PQ MOSTRA SPEEDUP NEGATIVO


## Por que repetir a chamada?

Uma única chamada `fib(25)` dura poucos milissegundos (frações de milissegundo na versão com corte) — tempo comparável ao ruído do sistema/agendador, não confiável numa leitura isolada de `omp_get_wtime`. Por isso os binários aceitam um segundo argumento (`repeticoes`): o cálculo é repetido várias vezes dentro da mesma região paralela e o tempo reportado já é a média.

## Como as diretivas impactam o desempenho

- O `shared` é necessário: sem ele, `i`/`j` seriam privadas à task e o resultado sairia errado.
- Cada `omp task` tem custo de criação/agendamento; para `fib(25)` isso são ~242 mil chamadas, a maioria trivial (`n<2`).
- Sem corte de granularidade, o overhead de criar cada task supera o trabalho que ela faz.
- Um `CUTOFF` que resolve casos pequenos sequencialmente elimina exatamente essas tasks inúteis.

## Resultados

![F(25)](fibonacci_f25.png)

| Threads | fib(25) sem corte | fib(25) com corte |
|---|---|---|
| 1 | 15,0 ms | 0,13 ms |
| 8 | 120,8 ms | 0,08 ms |

- **Sem corte, mais threads pioram o tempo** (até 10× mais lento com 8 threads) — cada chamada trivial vira uma task, e o overhead de agendar todas domina.
- **Com corte**, o tempo já cai ~150× com 1 thread, mas o ganho de threads adicionais é modesto (~1,7× em 4 threads): `CUTOFF=20` só deixa tasks nos níveis 21-25, pouco trabalho para dividir entre os núcleos.
- Conclusão: F(25) é quase um caso degenerado para tasking — o problema é pequeno demais para compensar paralelismo real, mesmo depois de corrigido o overhead de granularidade.

## Como rodar

```bash
./compilar.sh                            # gera ./fib_task e ./fib_task_cutoff
./bench_f25.sh > resultados_f25_baseline.csv          # F(25) sem corte, 1/2/4/8 threads
./bench_f25_cutoff.sh > resultados_f25_cutoff.csv     # F(25) com corte (CUTOFF=20)
python3 plot.py                          # gera fibonacci_f25.png (e fibonacci_sweep.png)
```
