#!/usr/bin/env python3
"""
Gera as curvas de desempenho (tempo x threads e speedup x threads) para os
3 experimentos da Tarefa 2 (Laplace 2D), a partir de um CSV de resultados,
mais um grafico combinado com todos os tempos.

Uso:
    python3 plot.py [arquivo.csv] [sufixo_saida]

Sem argumentos, le resultados.csv e grava curva_exp1_malha4096.png etc.
Com argumentos, ex.:
    python3 plot.py resultados_sdumont.csv _sdumont
grava curva_exp1_malha4096_sdumont.png etc. -- assim nao sobrescreve os
graficos gerados a partir de outro CSV/outra maquina. Se algum programa
nao existir no CSV (ex.: rodada parcial), o grafico que depende dele e
pulado em vez de travar o script.
"""
import csv
import statistics
import sys
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker

CSV_PATH = sys.argv[1] if len(sys.argv) > 1 else "resultados.csv"
SUFFIX = sys.argv[2] if len(sys.argv) > 2 else ""


def out(name):
    return f"{name}{SUFFIX}.png"


# programa -> threads -> [tempos]
data = defaultdict(lambda: defaultdict(list))
machines = set()

with open(CSV_PATH, newline="") as f:
    for row in csv.DictReader(f):
        data[row["programa"]][int(row["threads"])].append(float(row["tempo_s"]))
        if row.get("maquina"):
            machines.add(row["maquina"])

if len(machines) == 1:
    print(f"Dados de uma unica maquina: {next(iter(machines))}")
elif len(machines) > 1:
    print("AVISO: o CSV tem dados de MAIS DE UMA maquina -- os graficos vao")
    print("misturar/comparar tempos entre maquinas diferentes, o que nao faz sentido:")
    for m in sorted(machines):
        print(f"  - {m}")
else:
    print("AVISO: o CSV nao tem a coluna 'maquina' (dados de uma versao antiga do bench.sh)")


def mean_by_threads(prog):
    d = data[prog]
    threads = sorted(d.keys())
    means = [statistics.mean(d[t]) for t in threads]
    return threads, means


def speedup(threads, means):
    t1 = means[0]
    return [t1 / m for m in means]


def has(prog):
    if prog not in data:
        print(f"(pulando graficos que dependem de '{prog}': nao encontrado em {CSV_PATH})")
        return False
    return True


# Numero maximo de threads presente nos dados (nao mais fixo em 4 -- num
# cluster como o Santos Dumont pode ser 24, 48 etc.)
NCORES = max((t for prog in data for t in data[prog].keys()), default=4)


def style_x_threads(ax):
    ax.set_xlabel("threads (OMP_NUM_THREADS)")
    if NCORES <= 8:
        ax.set_xticks(range(1, NCORES + 1))
    else:
        ax.xaxis.set_major_locator(mticker.MaxNLocator(integer=True, nbins=10))


# Escala comum do eixo Y dos graficos de speedup: calculada a partir de
# todos os speedups de todos os experimentos presentes no CSV, para nao
# esconder o detalhe das curvas atras da linha de "speedup ideal" (removida).
ALL_PROGRAMS = ["laplace2d_gcc_i50", "laplace2d_gcc_i500",
                 "laplace2d_1k_gcc_i50", "laplace2d_1k_gcc_i500",
                 "laplace2d_1loop_gcc_i500"]

_all_speedups = []
for _prog in ALL_PROGRAMS:
    if _prog not in data:
        continue
    _threads, _means = mean_by_threads(_prog)
    _all_speedups.extend(speedup(_threads, _means))

if _all_speedups:
    _pad = 0.05 * (max(_all_speedups) - min(_all_speedups)) or 0.05
    SPEEDUP_YLIM = (min(_all_speedups) - _pad, max(_all_speedups) + _pad)
else:
    SPEEDUP_YLIM = (0, 2)


def style_speedup_axis(ax, title):
    ax.set_title(title)
    style_x_threads(ax)
    ax.set_ylabel("speedup (t1 / tN)")
    ax.set_ylim(*SPEEDUP_YLIM)
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)


# ---------------------------------------------------------------------
# Figura 1 - Experimento 1: laplace2d_gcc.c (malha 4096x4096), iter_max = 50 vs 500
# ---------------------------------------------------------------------
if has("laplace2d_gcc_i50") and has("laplace2d_gcc_i500"):
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

    for prog, label in [("laplace2d_gcc_i50", "iter_max=50"),
                         ("laplace2d_gcc_i500", "iter_max=500")]:
        threads, means = mean_by_threads(prog)
        axes[0].plot(threads, means, marker="o", label=label)
        axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

    axes[0].set_title("Experimento 1 — laplace2d_gcc.c (malha 4096x4096)\nTempo x threads")
    axes[0].set_ylabel("tempo (s)")
    style_x_threads(axes[0])
    axes[0].legend()
    axes[0].grid(alpha=0.3)

    style_speedup_axis(axes[1], "Speedup x threads")

    fig.tight_layout()
    fig.savefig(out("curva_exp1_malha4096"), dpi=150)
    plt.close(fig)

# ---------------------------------------------------------------------
# Figura 2 - Experimento 2: laplace2d_1k_gcc.c (malha 1024x1024), iter_max = 50 vs 500
#            (+ comparacao com a malha 4096 do experimento 1, mesmo iter_max)
# ---------------------------------------------------------------------
if has("laplace2d_1k_gcc_i50") and has("laplace2d_1k_gcc_i500"):
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

    for prog, label in [("laplace2d_1k_gcc_i50", "1024x1024, iter_max=50"),
                         ("laplace2d_1k_gcc_i500", "1024x1024, iter_max=500")]:
        threads, means = mean_by_threads(prog)
        axes[0].plot(threads, means, marker="o", label=label)
        axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

    if "laplace2d_gcc_i50" in data:
        # curva de referencia (malha 4096, mesmo iter_max) para comparar speedups
        threads, means = mean_by_threads("laplace2d_gcc_i50")
        axes[1].plot(threads, speedup(threads, means), marker="s", linestyle=":",
                     color="gray", label="ref.: 4096x4096, iter_max=50")

    axes[0].set_title("Experimento 2 — laplace2d_1k_gcc.c (malha 1024x1024)\nTempo x threads")
    axes[0].set_ylabel("tempo (s)")
    style_x_threads(axes[0])
    axes[0].legend()
    axes[0].grid(alpha=0.3)

    style_speedup_axis(axes[1], "Speedup x threads (malha pequena vs grande)")

    fig.tight_layout()
    fig.savefig(out("curva_exp2_malha1024"), dpi=150)
    plt.close(fig)

# ---------------------------------------------------------------------
# Figura 3 - Experimento 3: laplace2d_1loop_gcc.c (loops fundidos) vs
#            laplace2d_gcc_i500 (loops separados, mesma malha/iter_max)
# ---------------------------------------------------------------------
if has("laplace2d_gcc_i500") and has("laplace2d_1loop_gcc_i500"):
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

    for prog, label in [("laplace2d_gcc_i500", "loops separados (2 regioes/iter)"),
                         ("laplace2d_1loop_gcc_i500", "loops fundidos (1 regiao/iter)")]:
        threads, means = mean_by_threads(prog)
        axes[0].plot(threads, means, marker="o", label=label)
        axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

    axes[0].set_title("Experimento 3 — malha 4096x4096, iter_max=500\nTempo x threads")
    axes[0].set_ylabel("tempo (s)")
    style_x_threads(axes[0])
    axes[0].legend()
    axes[0].grid(alpha=0.3)

    style_speedup_axis(axes[1], "Speedup x threads (loops separados vs fundidos)")

    fig.tight_layout()
    fig.savefig(out("curva_exp3_fusao"), dpi=150)
    plt.close(fig)

# ---------------------------------------------------------------------
# Figura 4 - todos os tempos num so grafico (escala log no eixo Y)
# ---------------------------------------------------------------------
COMBINED_LABELS = [
    ("laplace2d_gcc_i50", "malha 4096, iter_max=50"),
    ("laplace2d_gcc_i500", "malha 4096, iter_max=500"),
    ("laplace2d_1k_gcc_i50", "malha 1024, iter_max=50"),
    ("laplace2d_1k_gcc_i500", "malha 1024, iter_max=500"),
    ("laplace2d_1loop_gcc_i500", "malha 4096, loops fundidos, iter_max=500"),
]
COMBINED_LABELS = [(p, l) for p, l in COMBINED_LABELS if p in data]

if COMBINED_LABELS:
    fig, ax = plt.subplots(figsize=(9.5, 5.5))

    for prog, label in COMBINED_LABELS:
        threads, means = mean_by_threads(prog)
        ax.plot(threads, means, marker="o", label=label)

    ax.set_yscale("log")
    ax.set_title("Todos os experimentos — Tempo x threads (escala log)")
    ax.set_ylabel("tempo (s, escala log)")
    style_x_threads(ax)
    ax.legend(fontsize=8, loc="center left", bbox_to_anchor=(1.02, 0.5))
    ax.grid(alpha=0.3, which="both")

    fig.tight_layout()
    fig.savefig(out("curva_todos_tempos"), dpi=150, bbox_inches="tight")
    plt.close(fig)

print(f"Graficos salvos com sufixo '{SUFFIX}' a partir de {CSV_PATH}.")
