# Tarefa 6 — Análise de `off_codigo3.c` (OpenMP Offload para GPU)

## 1. O que o código faz

`off_codigo3.c` demonstra o **gerenciamento explícito e não estruturado de dados**
entre host (CPU) e device (GPU) usando as diretivas de *offload* do OpenMP
(`target`). A ideia central é: em vez de copiar os arrays inteiros a cada
região `target` (o que geraria tráfego repetido pelo barramento PCIe/NVLink),
os dados são movidos para a GPU **uma única vez**, permanecem residentes lá
durante várias computações, e apenas as partes que realmente mudaram são
sincronizadas sob demanda.

Fluxo do programa:

1. Aloca e inicializa no host dois vetores de `double`, `host_source` (0, 1,
   2, …, 19) e `host_target` (todos -1.0).
2. `#pragma omp target enter data` — copia `host_source` para a GPU e apenas
   *aloca* espaço para `host_target` na GPU (sem copiar seu conteúdo).
3. Primeira região `#pragma omp target` — calcula `host_target[i] = host_source[i] * 2.0`
   **na GPU**, reaproveitando os buffers já mapeados no passo 2 (nenhuma
   nova transferência host→device acontece aqui).
4. `#pragma omp target update from` — traz o `host_target` recém-calculado
   de volta para o host, só para visualização, sem desalocar nada na GPU.
5. O host modifica localmente `host_source[0] = 100.0` e usa
   `#pragma omp target update to(host_source[0:1])` para sincronizar
   **apenas esse elemento** de volta para a GPU.
6. Segunda região `#pragma omp target` — recalcula `host_target[i] = host_source[i] + 5.0`
   na GPU, já usando o valor atualizado de `host_source[0]`.
7. `#pragma omp target exit data` — traz o `host_target` final de volta ao
   host (`map(from:)`) e libera a memória de `host_source` na GPU
   (`map(delete:)`, sem copiar de volta, pois não mudou mais na GPU).
8. O host imprime e valida os resultados finais.

## 2. Diretivas utilizadas (foco em GPU)

| Diretiva | Papel no offload | Observação de otimização |
|---|---|---|
| `#pragma omp target enter data map(to:...) map(alloc:...)` | Abre uma **região de dados não estruturada**: aloca memória no device e, opcionalmente, copia dados host→device, **sem** disparar execução de código. | `map(to:)` copia (necessário para `host_source`, que a GPU precisa ler); `map(alloc:)` só reserva espaço (não copia `host_target`, pois seu valor inicial de -1.0 é irrelevante — evita cópia desnecessária). |
| `#pragma omp target` (sem `map`) | Executa o laço no device. | Como os vetores já estão mapeados (passo anterior), o runtime usa a semântica *present* — reaproveita os ponteiros de device já existentes, sem nova alocação/cópia. |
| `#pragma omp target update from(...)` | Sincroniza **device → host** um trecho específico dos dados, sem encerrar o ciclo de vida do mapeamento. | Permite inspecionar/usar um resultado intermediário no host mantendo os dados vivos na GPU para as próximas regiões `target` (evita ter que remapear tudo de novo). |
| `#pragma omp target update to(...)` | Sincroniza **host → device** um trecho específico dos dados. | Aqui é usado `host_source[0:1]` (1 elemento) em vez de `host_source[0:N]` (20 elementos) — atualiza só o que mudou, minimizando o volume de dados movido. |
| `#pragma omp target exit data map(from:...) map(delete:...)` | Fecha o ciclo de vida dos dados no device: `map(from:)` copia device→host e libera; `map(delete:)` só libera, sem copiar de volta. | `host_source` é apenas `delete`d (não `from`) porque seu valor na GPU não mudou desde o último `update to` — copiar de volta seria banda desperdiçada. |

Todas essas diretivas fazem parte do modelo de **dados não estruturados**
(*unstructured data lifetime*) do OpenMP, que se opõe ao modelo *estruturado*
(uma única cláusula `map` no próprio `#pragma omp target`, como em
`off_codigo1.c`, cujo escopo de vida dos dados coincide exatamente com o
bloco `{ }` da região target). O modelo não estruturado é o que permite,
neste código, **múltiplas regiões `target` compartilharem os mesmos buffers
de GPU** sem retransferir os arrays inteiros a cada chamada.

### Cláusulas de `map` usadas

- `map(to: x[0:N])` — host → device, só na entrada (é o que a GPU precisa ler).
- `map(from: x[0:N])` — device → host, só na saída (é o resultado que o host precisa ler).
- `map(alloc: x[0:N])` — apenas reserva memória no device, sem transferir conteúdo.
- `map(delete: x[0:N])` — libera a memória do device, sem transferir conteúdo de volta.

## 3. Otimizações de desempenho evidenciadas

1. **Persistência de dados no device** (`target enter data` / `target exit data`):
   evita que cada uma das duas regiões `target` recopie `host_source` e
   `host_target` inteiros — a cópia de `host_source` acontece 1 vez (20
   elementos) em vez de 2 vezes (40 elementos no total), e `host_target`
   nunca precisa ser copiado host→device.
2. **Atualização parcial/seletiva** (`target update to(host_source[0:1])`):
   ao mudar só `host_source[0]`, o programa sincroniza 1 `double` (8 bytes)
   em vez do vetor inteiro (160 bytes) — evidencia a prática de minimizar o
   volume de dados movidos pelo barramento host↔device, tipicamente o maior
   gargalo em aplicações de GPU (muito mais lento que a memória da própria
   GPU ou o cache da CPU).
3. **`map(alloc:)` em vez de `map(to:)` para `host_target`**: como o valor
   inicial (-1.0) nunca é lido pela GPU (é sobrescrito antes de ser usado),
   copiá-lo seria desperdício — só a alocação de espaço é necessária.
4. **`map(delete:)` em vez de `map(from:)` para `host_source` no `exit data`**:
   evita copiar de volta um dado que o host já possui de forma equivalente
   (o host já tem o valor mais atual de `host_source`, já que só ele mesmo
   o modificou por último).

> Observação: o código **não** usa `teams distribute parallel for` dentro das
> regiões `target` — ou seja, o laço roda em uma única *team*/thread na GPU
> (correto, mas sem exploração do paralelismo massivo do device). O foco
> deste exemplo é exclusivamente a **estratégia de movimentação de dados**,
> não a paralelização do cômputo em si.

## 4. Compilação e execução

Arquivos auxiliares neste diretório, no mesmo padrão dos demais exemplos do
curso (scripts `compilar.sh` + `run.sh` com SLURM/`sbatch`):

```bash
# No cluster (SDumont ou similar, com módulo NVHPC/PGI disponível):
bash compilar.sh
sbatch run.sh ./off_codigo3
```

`compilar.sh` usa `nvc -mp=gpu -Minfo=mp` (NVHPC, sucessor do PGI) para gerar
código de offload para GPU NVIDIA; `-Minfo=mp` imprime no terminal quais
regiões o compilador de fato conseguiu mapear para o device — útil para
confirmar que o `#pragma omp target` realmente gerou código de GPU e não
caiu silenciosamente para execução no host.

**Nota sobre este ambiente de análise:** este relatório foi preparado em uma
máquina Windows sem GPU e sem toolchain OpenMP-offload instalada, portanto a
execução real em GPU não pôde ser feita aqui. A seção abaixo mostra a saída
**esperada**, obtida por rastreamento manual (*trace*) do código — o próprio
programa já inclui *asserts* impressos (`Esperado: ...`) que conferem esse
resultado quando executado de fato no cluster.

## 5. Execução — saída esperada (trace manual)

Estado inicial: `host_source = {0, 1, 2, 3, 4, ..., 19}`, `host_target = {-1, -1, ..., -1}`.

```
Dados inciados no hospedeiro.
Dados mapeados/alocados no dispositivo.
host_target modificado no dispositivo.
Hospedeiro: host_target (após update from) (primeiros 5 elementos): { 0.00, 2.00, 4.00, 6.00, 8.00 }
Hospedeiro: host_source (após modificação local) (primeiros 5 elementos): { 100.00, 1.00, 2.00, 3.00, 4.00 }
host_source[0] atualizado do hospedeiro para o dispositivo.
Segunda computação modificando host_target no dispositivo.
Dados finais de host_target transferidos para o hospedeiro, memória liberada no dispositivo.
Verificação final dos dados no Hospedeiro:
Hospedeiro: host_source (final) (primeiros 5 elementos): { 100.00, 1.00, 2.00, 3.00, 4.00 }
Hospedeiro: host_target (final) (primeiros 5 elementos): { 105.00, 6.00, 7.00, 8.00, 9.00 }
Verificação de host_target[0]: 105.00 (Esperado: 100.0 + 5.0 = 105.0)
Verificação de host_target[1]: 6.00 (Esperado: 1.0 + 5.0 = 6.0)
```

Passo a passo do porquê desses valores:

- Após a **1ª região target**, `host_target[i] = host_source[i] * 2.0` para
  `i = 0..19`, com `host_source` ainda igual a `{0, 1, 2, ...}` (cópia
  original enviada pelo `enter data`). Por isso `update from` traz
  `{0.00, 2.00, 4.00, 6.00, 8.00, ...}`.
- O host então muda **sua própria cópia local** de `host_source[0]` para
  `100.0` — isso **não** afeta automaticamente a GPU (os dois lados só
  ficam sincronizados por causa das diretivas `target update` explícitas).
- `target update to(host_source[0:1])` propaga só esse elemento para a GPU;
  os demais elementos de `host_source` na GPU continuam `1, 2, 3, ..., 19`.
- Na **2ª região target**, `host_target[i] = host_source[i] + 5.0`. Como na
  GPU `host_source[0] == 100.0` (atualizado) e `host_source[1..19]` seguem
  `1..19` (nunca atualizados), o resultado é `host_target = {105, 6, 7, 8, 9, ..., 24}`.
- `target exit data` traz esse `host_target` final para o host e libera os
  dois buffers na GPU — encerrando o ciclo de vida do offload.

## 6. Se executar no cluster

Para confirmar a saída real em GPU e comparar com o trace acima, execute:

```bash
sbatch run.sh ./off_codigo3
cat slurm-<job_id>.out
```

e verifique se as duas linhas de "Verificação de host_target" batem com os
valores esperados impressos pelo próprio programa — isso confirma que o
gerenciamento não-estruturado de dados (`enter data` / `update` / `exit data`)
funcionou como projetado, sem nenhuma cópia supérflua entre host e device.
