#!/usr/bin/env python3
"""Curva de desempenho do produto escalar por estrategia de sincronizacao.

Le resultados.csv (colunas: threads,versao,tempo_s) e gera
grafico_escalar.png com dois paineis: tempo x threads (escala log) e
speedup x threads, uma linha por versao (critical/atomic/local/reduction).
"""
import csv
import matplotlib.pyplot as plt

CORES = {
    "critical": "#d64550",
    "atomic": "#e8a33d",
    "local": "#2f6690",
    "reduction": "#3ea55f",
}
ORDEM = ["critical", "atomic", "local", "reduction"]


def ler_csv(caminho):
    linhas = []
    with open(caminho, newline="") as f:
        for row in csv.DictReader(f):
            linhas.append(
                {"threads": int(row["threads"]), "versao": row["versao"], "tempo": float(row["tempo_s"])}
            )
    return linhas


def serie(dados, versao):
    filtrado = sorted((d for d in dados if d["versao"] == versao), key=lambda d: d["threads"])
    return [d["threads"] for d in filtrado], [d["tempo"] for d in filtrado]


def main():
    dados = ler_csv("resultados.csv")

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
    fig.suptitle("Produto escalar — critical x atomic x local x reduction", fontsize=12)

    for versao in ORDEM:
        x, y = serie(dados, versao)
        ax1.plot(x, y, marker="o", color=CORES[versao], label=versao)
    ax1.set_yscale("log")
    ax1.set_xticks([1, 2, 4, 8])
    ax1.set_xlabel("threads (OMP_NUM_THREADS)")
    ax1.set_ylabel("tempo (s, escala log)")
    ax1.set_title("tempo x threads (N=20.000.000)")
    ax1.grid(True, which="both", alpha=0.3)
    ax1.legend(fontsize=8)

    for versao in ORDEM:
        x, y = serie(dados, versao)
        t1 = y[x.index(1)]
        speedup = [t1 / t for t in y]
        ax2.plot(x, speedup, marker="o", color=CORES[versao], label=versao)
    ax2.axhline(1.0, linestyle="--", color="#999999", linewidth=1, label="sem ganho (speedup=1)")
    ax2.set_xticks([1, 2, 4, 8])
    ax2.set_xlabel("threads (OMP_NUM_THREADS)")
    ax2.set_ylabel("speedup (T1/Tn)")
    ax2.set_title("speedup x threads")
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=8)

    fig.tight_layout(rect=[0, 0, 1, 0.94])
    fig.savefig("grafico_escalar.png", dpi=150)
    print("salvo: grafico_escalar.png")


if __name__ == "__main__":
    main()
