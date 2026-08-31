# Apresentação — Tarefa 6: offload para GPU (análise, sem execução local)

Esta máquina não tem GPU nem toolchain de offload — a análise abaixo é por
inspeção de código (*trace* manual), sem execução real. Ver `README.md` para
o passo a passo completo do trace.

## Arquivos

- `off_codigo1.c` — offload **estruturado**: uma única `#pragma omp target map(...)` cujo escopo de dados coincide com o bloco `{ }` da região. Demonstra `map(to:)`, `map(from:)`, `firstprivate`, `private` e a cláusula `if` (cai pro host se não há dispositivo).
- `off_codigo3.c` — offload **não estruturado**: `#pragma omp target enter data` / `target update to/from` / `target exit data` mantêm os vetores residentes na GPU entre duas regiões `target`, sincronizando só o que muda.
- `compilar.sh` — compila com `nvc -mp=gpu -Minfo=mp` (NVHPC), pra rodar num cluster com GPU (ex.: SDumont).
- `run.sh` — script de submissão SLURM (`sbatch run.sh ./programa`).

## Como as diretivas impactam o desempenho

- Cada cópia host↔device passa pelo barramento PCIe, tipicamente o maior gargalo de uma aplicação de GPU — muito mais lento que a memória da própria GPU.
- `target enter data` / `exit data` deixam os dados residentes na GPU entre várias regiões `target`, evitando recopiar vetores inteiros a cada chamada.
- `target update to/from` sincroniza só o trecho que mudou (ex.: 1 elemento em vez do vetor inteiro) — minimiza volume de dados movido.
- `map(alloc:)` reserva espaço sem copiar (usado quando o valor inicial não importa); `map(delete:)` libera sem copiar de volta (usado quando o host já tem o dado atualizado).

## Resultado esperado (`off_codigo3.c`, trace manual)

| Passo | `host_source[0]` | `host_target[0]` |
|---|---|---|
| após 1ª região target (`×2`) | 0 | 0 |
| host muda localmente, `update to` propaga só esse elemento | 100 | — |
| após 2ª região target (`+5`) | 100 | **105** |

- O host muda `host_source[0]` **localmente** e isso não afeta a GPU até o `target update to` explícito — evidencia que host e device são memórias independentes, sincronizadas só quando pedido.
- `host_target[0]` final é `105` (`100+5`), não `2×0+5`, confirmando que a 2ª região realmente leu o valor atualizado da GPU, não uma cópia obsoleta.
- Esse padrão (dados residentes + atualização seletiva) evita 3 das 4 cópias completas de vetor que uma versão ingênua (um `map` por região `target`) faria.

## Como rodar (só em cluster com GPU, ex.: SDumont)

```bash
./compilar.sh                    # module load nvhpc; nvc -mp=gpu -Minfo=mp ...
sbatch run.sh ./off_codigo1        # submete via SLURM
sbatch run.sh ./off_codigo3
```
