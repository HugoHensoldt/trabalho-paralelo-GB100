#!/usr/bin/env python3
"""
Gera as curvas de desempenho (tempo x threads e speedup x threads) para os
3 experimentos da Tarefa 2 (Laplace 2D), a partir de resultados.csv, mais
um grafico combinado com todos os tempos.
"""
import csv
import statistics
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

CSV_PATH = "resultados.csv"

# programa -> threads -> [tempos]
data = defaultdict(lambda: defaultdict(list))

with open(CSV_PATH, newline="") as f:
    for row in csv.DictReader(f):
        data[row["programa"]][int(row["threads"])].append(float(row["tempo_s"]))


def mean_by_threads(prog):
    d = data[prog]
    threads = sorted(d.keys())
    means = [statistics.mean(d[t]) for t in threads]
    return threads, means


def speedup(threads, means):
    t1 = means[0]
    return [t1 / m for m in means]


NCORES = 4

# Escala comum do eixo Y dos graficos de speedup: calculada a partir de
# todos os speedups de todos os experimentos, para nao esconder o detalhe
# das curvas atras da linha de "speedup ideal" (removida abaixo).
ALL_PROGRAMS = ["laplace2d_gcc_i50", "laplace2d_gcc_i500",
                 "laplace2d_1k_gcc_i50", "laplace2d_1k_gcc_i500",
                 "laplace2d_1loop_gcc_i500"]

_all_speedups = []
for _prog in ALL_PROGRAMS:
    _threads, _means = mean_by_threads(_prog)
    _all_speedups.extend(speedup(_threads, _means))

_pad = 0.05 * (max(_all_speedups) - min(_all_speedups))
SPEEDUP_YLIM = (min(_all_speedups) - _pad, max(_all_speedups) + _pad)


def style_speedup_axis(ax, title):
    ax.set_title(title)
    ax.set_xlabel("threads (OMP_NUM_THREADS)")
    ax.set_ylabel("speedup (t1 / tN)")
    ax.set_xticks(range(1, NCORES + 1))
    ax.set_ylim(*SPEEDUP_YLIM)
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)


# ---------------------------------------------------------------------
# Figura 1 - Experimento 1: laplace2d_gcc.c (malha 4096x4096), iter_max = 50 vs 500
# ---------------------------------------------------------------------
fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

for prog, label in [("laplace2d_gcc_i50", "iter_max=50"),
                     ("laplace2d_gcc_i500", "iter_max=500")]:
    threads, means = mean_by_threads(prog)
    axes[0].plot(threads, means, marker="o", label=label)
    axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

axes[0].set_title("Experimento 1 — laplace2d_gcc.c (malha 4096x4096)\nTempo x threads")
axes[0].set_xlabel("threads (OMP_NUM_THREADS)")
axes[0].set_ylabel("tempo (s)")
axes[0].set_xticks(range(1, NCORES + 1))
axes[0].legend()
axes[0].grid(alpha=0.3)

style_speedup_axis(axes[1], "Speedup x threads")

fig.tight_layout()
fig.savefig("curva_exp1_malha4096.png", dpi=150)
plt.close(fig)

# ---------------------------------------------------------------------
# Figura 2 - Experimento 2: laplace2d_1k_gcc.c (malha 1024x1024), iter_max = 50 vs 500
#            (+ comparacao com a malha 4096 do experimento 1, mesmo iter_max)
# ---------------------------------------------------------------------
fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

for prog, label in [("laplace2d_1k_gcc_i50", "1024x1024, iter_max=50"),
                     ("laplace2d_1k_gcc_i500", "1024x1024, iter_max=500")]:
    threads, means = mean_by_threads(prog)
    axes[0].plot(threads, means, marker="o", label=label)
    axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

# curva de referencia (malha 4096, mesmo iter_max) para comparar speedups
threads, means = mean_by_threads("laplace2d_gcc_i50")
axes[1].plot(threads, speedup(threads, means), marker="s", linestyle=":",
             color="gray", label="ref.: 4096x4096, iter_max=50")

axes[0].set_title("Experimento 2 — laplace2d_1k_gcc.c (malha 1024x1024)\nTempo x threads")
axes[0].set_xlabel("threads (OMP_NUM_THREADS)")
axes[0].set_ylabel("tempo (s)")
axes[0].set_xticks(range(1, NCORES + 1))
axes[0].legend()
axes[0].grid(alpha=0.3)

style_speedup_axis(axes[1], "Speedup x threads (malha pequena vs grande)")

fig.tight_layout()
fig.savefig("curva_exp2_malha1024.png", dpi=150)
plt.close(fig)

# ---------------------------------------------------------------------
# Figura 3 - Experimento 3: laplace2d_1loop_gcc.c (loops fundidos) vs
#            laplace2d_gcc_i500 (loops separados, mesma malha/iter_max)
# ---------------------------------------------------------------------
fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))

for prog, label in [("laplace2d_gcc_i500", "loops separados (2 regioes/iter)"),
                     ("laplace2d_1loop_gcc_i500", "loops fundidos (1 regiao/iter)")]:
    threads, means = mean_by_threads(prog)
    axes[0].plot(threads, means, marker="o", label=label)
    axes[1].plot(threads, speedup(threads, means), marker="o", label=label)

axes[0].set_title("Experimento 3 — malha 4096x4096, iter_max=500\nTempo x threads")
axes[0].set_xlabel("threads (OMP_NUM_THREADS)")
axes[0].set_ylabel("tempo (s)")
axes[0].set_xticks(range(1, NCORES + 1))
axes[0].legend()
axes[0].grid(alpha=0.3)

style_speedup_axis(axes[1], "Speedup x threads (loops separados vs fundidos)")

fig.tight_layout()
fig.savefig("curva_exp3_fusao.png", dpi=150)
plt.close(fig)

# ---------------------------------------------------------------------
# Figura 4 - todos os tempos num so grafico (escala log no eixo Y: os
# tempos variam de ~0.13s ate ~31s entre os experimentos)
# ---------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(9.5, 5.5))

COMBINED_LABELS = [
    ("laplace2d_gcc_i50", "malha 4096, iter_max=50"),
    ("laplace2d_gcc_i500", "malha 4096, iter_max=500"),
    ("laplace2d_1k_gcc_i50", "malha 1024, iter_max=50"),
    ("laplace2d_1k_gcc_i500", "malha 1024, iter_max=500"),
    ("laplace2d_1loop_gcc_i500", "malha 4096, loops fundidos, iter_max=500"),
]

for prog, label in COMBINED_LABELS:
    threads, means = mean_by_threads(prog)
    ax.plot(threads, means, marker="o", label=label)

ax.set_yscale("log")
ax.set_title("Todos os experimentos — Tempo x threads (escala log)")
ax.set_xlabel("threads (OMP_NUM_THREADS)")
ax.set_ylabel("tempo (s, escala log)")
ax.set_xticks(range(1, NCORES + 1))
ax.legend(fontsize=8, loc="center left", bbox_to_anchor=(1.02, 0.5))
ax.grid(alpha=0.3, which="both")

fig.tight_layout()
fig.savefig("curva_todos_tempos.png", dpi=150, bbox_inches="tight")
plt.close(fig)

print("Graficos salvos: curva_exp1_malha4096.png, curva_exp2_malha1024.png, "
      "curva_exp3_fusao.png, curva_todos_tempos.png")
