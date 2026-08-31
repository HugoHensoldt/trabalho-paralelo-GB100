# Apresentação — Tarefa 1: multiplicação de matrizes e `schedule`

## Arquivos

- `run_benchmark.sh — roda as 6 combinações (`static|dynamic|guided` × `chunk=2|100`), 
- `mult_matriz.c` — multiplica mattriz com `for schedule(runtime)`, o par `(modo, chunk)` vem do .sh `OMP_SCHEDULE`, sem recompilar.


## Como o `schedule` impacta o desempenho

- Cada iteração do laço externo faz a mesma quantidade de trabalho (uma linha inteira de `C`) — carga já balanceada entre iterações.
- `static` divide o laço em blocos fixos decididos antes de começar, sem coordenação em tempo real → overhead mínimo.
- `dynamic` busca o próximo chunk sob demanda (seção crítica implícita a cada chunk) — só compensa esse custo quando há desbalanceamento real de carga, que não é o caso aqui.
- `guided` começa com chunks grandes e diminui gradualmente — `chunk` vira um tamanho *mínimo*, não o tamanho de todo chunk, por isso tolera bem valores pequenos.

## Como cada modo distribui as linhas entre as threads

Ilustração com N=4096 linhas e 4 threads, chunk=100 (com chunk=2 o padrão é
o mesmo, só que em blocos de 2 em vez de 100 — 2048 blocos disputando
despacho em vez de 41).

**`static,100`** — blocos fixos, distribuídos em round-robin, decidido
**antes** do laço começar (sem coordenação em tempo de execução, por isso é
sempre igual e determinístico):
```
thread 0: linhas 0–99,    400–499, 800–899, ...
thread 1: linhas 100–199, 500–599, 900–999, ...
thread 2: linhas 200–299, 600–699, 1000–1099, ...
thread 3: linhas 300–399, 700–799, 1100–1199, ...
```

**`dynamic,100`** — cada thread livre pega o próximo bloco disponível assim
que termina o anterior; a ordem só se conhece em tempo de execução (não é
fixa, depende de quem fica ociosa primeiro):
```
thread 2: linhas 0–99      (a primeira a pedir trabalho)
thread 0: linhas 100–199
thread 3: linhas 200–299
thread 1: linhas 300–399
thread 2: linhas 400–499   (thread 2 já terminou seu bloco e pega outro)
...
```

**`guided,100`** — o *tamanho* de cada bloco é determinístico (fórmula
`tamanho = teto(restante / threads)`, decaindo a cada despacho até bater no
mínimo, que é o chunk); só *qual thread pega qual bloco* é decidido em
tempo de execução:
```
bloco 1:  linhas 0–1023      (1024 linhas = teto(4096/4))
bloco 2:  linhas 1024–1791   (768 linhas  = teto(3072/4))
bloco 3:  linhas 1792–2367   (576 linhas  = teto(2304/4))
bloco 4:  linhas 2368–2799   (432 linhas  = teto(1728/4))
bloco 5:  linhas 2800–3123   (324 linhas  = teto(1296/4))
bloco 6:  linhas 3124–3366   (243 linhas  = teto(972/4))
bloco 7:  linhas 3367–3549   (183 linhas  = teto(729/4))
bloco 8:  linhas 3550–3686   (137 linhas  = teto(546/4))
bloco 9:  linhas 3687–3789   (103 linhas  = teto(409/4))
bloco 10: linhas 3790–3889   (100 linhas — teto(306/4)=77 seria menor que o
                               mínimo, então trava em 100)
bloco 11: linhas 3890–3989   (100 linhas, no mínimo)
bloco 12: linhas 3990–4089   (100 linhas, no mínimo)
bloco 13: linhas 4090–4095   (6 linhas — resto final, menor que o mínimo)
```
Os 13 blocos vão sendo entregues, um de cada vez, para a próxima thread que
ficar livre — por isso a mesma thread pode pegar mais de um bloco (ex.:
quem pegou o bloco 1 pode voltar e pegar o bloco 5 também).

## Resultados (4 threads, média de 3 execuções)

| schedule | chunk=2    | chunk=100 |
|--------  |----------- |-----------|
| static   | 25,57 s    | 25,35 s   |
| dynamic  | 32,82 s    | 27,72 s   |
| guided   | **23,23 s**| 25,61 s   |

- `dynamic,2` foi o pior (32,82 s): 2048 chunks, cada um pagando sincronização sem necessidade, já que não há desbalanceamento para corrigir.
- `guided,2` foi o melhor (23,23 s): talvez pela menor sensibilidade a variação de velocidade entre threads durante a execução.
- `static` foi o mais estável entre `chunk=2` e `chunk=100` — coerente com não depender de coordenação por chunk.

- Conclusão prática: `dynamic` só vale a pena com desbalanceamento real de trabalho entre iterações. 'guided' em processamento não dedicado

## Como rodar

```bash
./compilar.sh          # gera ./mult_matriz (rodado automaticamente se faltar)
./run_benchmark.sh 4 3 # OMP_NUM_THREADS=4, 3 repetições -- gera resultados.csv
```
