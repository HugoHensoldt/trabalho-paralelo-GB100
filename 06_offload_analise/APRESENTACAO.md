# Apresentação — Tarefa 6: gerenciamento de dados no offload para GPU

Análise de `off_codigo3.c`: gerenciamento **explícito e não estruturado** de
dados entre host e GPU via `#pragma omp target enter data / update / exit
data`. Compilado e executado no SDumont (job GPU via SLURM) — resultado real
confirmado, ver abaixo.

## Arquivos

- `off_codigo1.c` — offload **estruturado**: um único `#pragma omp target map(...)` cujo escopo de dados coincide com o bloco `{ }` da região (contraste com o modelo abaixo).
- `off_codigo3.c` — offload **não estruturado**: `target enter data` / `target update to/from` / `target exit data` mantêm os vetores residentes na GPU entre duas regiões `target`, sincronizando só o que muda.
- `compilar.sh` — compila com `nvc -mp=gpu -Minfo=mp` (NVHPC), pra rodar num cluster com GPU (ex.: SDumont).
- `run.sh` — script de submissão SLURM (`sbatch run.sh ./programa`), fila `sequana_gpu_dev`.

## Como as diretivas impactam o desempenho

- Cada cópia host↔device passa pelo barramento PCIe, tipicamente o maior gargalo de uma aplicação de GPU — muito mais lento que a memória da própria GPU.
- `target enter data` / `exit data` deixam os dados residentes na GPU entre várias regiões `target`, evitando recopiar vetores inteiros a cada chamada.
- `target update to/from` sincroniza só o trecho que mudou (ex.: 1 elemento em vez do vetor inteiro) — minimiza volume de dados movido.
- `map(alloc:)` reserva espaço sem copiar (usado quando o valor inicial não importa); `map(delete:)` libera sem copiar de volta (usado quando o host já tem o dado atualizado).

## Como compilar e rodar no SDumont

```bash
bash compilar.sh                  # module load nvhpc; nvc -mp=gpu -Minfo=mp ...
sbatch run.sh ./off_codigo3        # submete via SLURM (fila com GPU)
cat slurm-<job_id>.out
```

## Resultado real (SDumont, job 11589424)

```bash
[hugo.hensoldt@sdumont17 06_offload_analise]$ sbatch run.sh ./off_codigo3
Submitted batch job 11589424
[hugo.hensoldt@sdumont17 06_offload_analise]$ cat slurm-11589424.out
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

| Passo                                  | `host_source[0]` na GPU | `host_target[0]` |
|------------------------------------------------------------|-----|------------------|
| após 1ª região target (`×2`)                               | 0   | 0.00             |
| host muda localmente, `update to` propaga só esse elemento | 100 | —                |
| após 2ª região target (`+5`)                               | 100 | **105.00**       |

- O host muda `host_source[0]` **localmente** e isso não afeta a GPU até o `target update to` explícito — a saída real confirma que os 5 primeiros elementos de `host_target` após a 1ª região (`0, 2, 4, 6, 8`) usam a cópia original de `host_source` na GPU, não o valor 100 já presente no host.
- `host_target[0]` final é `105.00` (`100+5`), não `2×0+5=0`, confirmando que a 2ª região realmente leu o valor atualizado da GPU (propagado pelo `target update to`), não uma cópia obsoleta.
- Esse padrão (dados residentes + atualização seletiva) evita 3 das 4 cópias completas de vetor que uma versão ingênua (um `map` por região `target`) faria — e a execução real bateu, elemento a elemento, com o previsto por inspeção do código.
