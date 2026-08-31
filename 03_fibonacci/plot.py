#!/usr/bin/env python3
"""Curvas de desempenho do Fibonacci paralelo (omp task).

Gera dois PNGs:

1) fibonacci_f25.png — F(25) especificamente. Cada chamada e rapida demais
   para uma leitura de tempo confiavel isolada, entao o driver C repete o
   calculo internamente (REPETICOES) e o tempo reportado ja e a media.
   Le resultados_f25_baseline.csv / resultados_f25_cutoff.csv.

2) fibonacci_sweep.png — varredura de n (28..44), onde uma unica chamada
   ja dura de dezenas de ms a segundos (sem necessidade de repeticao
   interna). Mostra tempo x threads (n=34) e speedup x threads (versao
   com corte) para alguns n.
   Le resultados_sweep_baseline.csv / resultados_sweep_cutoff.csv.
"""
import csv
import matplotlib.pyplot as plt

COR_BASELINE = "#d64550"
COR_CUTOFF = "#2f6690"
CORES_N = {34: "#8fb339", 40: "#2f6690", 44: "#d64550"}


def ler_csv(caminho):
    linhas = []
    with open(caminho, newline="") as f:
        for row in csv.DictReader(f):
            linhas.append(
                {"threads": int(row["threads"]), "n": int(row["n"]), "tempo": float(row["tempo_s"])}
            )
    return linhas


def serie(dados, n_alvo):
    filtrado = sorted((d for d in dados if d["n"] == n_alvo), key=lambda d: d["threads"])
    return [d["threads"] for d in filtrado], [d["tempo"] for d in filtrado]


def plot_f25():
    baseline = ler_csv("resultados_f25_baseline.csv")
    cutoff = ler_csv("resultados_f25_cutoff.csv")

    xb, yb = serie(baseline, 25)
    xc, yc = serie(cutoff, 25)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
    fig.suptitle("Fibonacci recursivo com omp task — F(25) (media de varias repeticoes)", fontsize=12)

    ax1.plot(xb, yb, marker="o", color=COR_BASELINE, label="sem corte (task por chamada)")
    ax1.plot(xc, yc, marker="o", color=COR_CUTOFF, label="com corte (CUTOFF=20)")
    ax1.set_yscale("log")
    ax1.set_xticks([1, 2, 4, 8])
    ax1.set_xlabel("threads (OMP_NUM_THREADS)")
    ax1.set_ylabel("tempo medio por chamada (s, escala log)")
    ax1.set_title("tempo x threads")
    ax1.grid(True, which="both", alpha=0.3)
    ax1.legend(fontsize=8)

    sb = [yb[0] / t for t in yb]
    sc = [yc[0] / t for t in yc]
    ax2.plot(xb, sb, marker="o", color=COR_BASELINE, label="sem corte")
    ax2.plot(xc, sc, marker="o", color=COR_CUTOFF, label="com corte")
    ax2.axhline(1.0, linestyle="--", color="#999999", linewidth=1, label="sem ganho (speedup=1)")
    ax2.set_xticks([1, 2, 4, 8])
    ax2.set_xlabel("threads (OMP_NUM_THREADS)")
    ax2.set_ylabel("speedup (T1/Tn)")
    ax2.set_title("speedup x threads")
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=8)

    fig.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig("fibonacci_f25.png", dpi=150)
    print("salvo: fibonacci_f25.png")


def plot_sweep():
    baseline = ler_csv("resultados_sweep_baseline.csv")
    cutoff = ler_csv("resultados_sweep_cutoff.csv")

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
    fig.suptitle("Fibonacci recursivo com omp task — varredura de n (28-44)", fontsize=12)

    N_COMPARACAO = 34
    xt, yt = serie(baseline, N_COMPARACAO)
    ax1.plot(xt, yt, marker="o", color=COR_BASELINE, label="sem corte (task por chamada)")
    xt2, yt2 = serie(cutoff, N_COMPARACAO)
    ax1.plot(xt2, yt2, marker="o", color=COR_CUTOFF, label="com corte (CUTOFF=20)")
    ax1.set_yscale("log")
    ax1.set_xticks([1, 2, 4, 8])
    ax1.set_xlabel("threads (OMP_NUM_THREADS)")
    ax1.set_ylabel("tempo (s, escala log)")
    ax1.set_title(f"tempo x threads (n={N_COMPARACAO})")
    ax1.grid(True, which="both", alpha=0.3)
    ax1.legend(fontsize=8)

    for n_val in (34, 40, 44):
        xs, ys = serie(cutoff, n_val)
        t1 = ys[xs.index(1)]
        speedup = [t1 / t for t in ys]
        ax2.plot(xs, speedup, marker="o", color=CORES_N[n_val], label=f"n={n_val}")
    ax2.plot([1, 8], [1, 8], linestyle="--", color="#999999", linewidth=1, label="speedup ideal")
    ax2.set_xticks([1, 2, 4, 8])
    ax2.set_xlabel("threads (OMP_NUM_THREADS)")
    ax2.set_ylabel("speedup (T1/Tn)")
    ax2.set_title("speedup x threads (versao com corte)")
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=8)

    fig.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig("fibonacci_sweep.png", dpi=150)
    print("salvo: fibonacci_sweep.png")


if __name__ == "__main__":
    plot_f25()
    plot_sweep()
