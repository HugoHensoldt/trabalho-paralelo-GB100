# Le os 3 CSVs gerados pelos jobs sbatch (resultados_openmp.csv,
# resultados_mpi.csv, resultados_hibrido.csv), calcula a media de tempo por
# configuracao (1 a 6 processos/threads) e plota as 3 curvas juntas.
#
# Rodar depois que os 3 jobs (bench_openmp.sh, bench_mpi.sh, bench_hybrid.sh)
# tiverem terminado no SDumont e os CSVs estiverem neste diretorio.

import csv
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def medias_por_config(caminho_csv, coluna_config):
    soma = defaultdict(float)
    conta = defaultdict(int)
    with open(caminho_csv, newline="") as f:
        for linha in csv.DictReader(f):
            k = int(linha[coluna_config])
            soma[k] += float(linha["tempo_s"])
            conta[k] += 1
    return {k: soma[k] / conta[k] for k in soma}


openmp = medias_por_config("resultados_openmp.csv", "threads")
mpi = medias_por_config("resultados_mpi.csv", "procs")
hibrido = medias_por_config("resultados_hibrido.csv", "threads")

x = sorted(openmp.keys())

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))

ax1.plot(x, [openmp[k] for k in x], marker='o', label='OpenMP puro (1 no)')
ax1.plot(x, [mpi[k] for k in x], marker='s', label='MPI puro (2 nos)')
ax1.plot(x, [hibrido[k] for k in x], marker='^', label='Hibrido MPI+OpenMP (2 nos)')
ax1.set_xlabel('Numero de processos/threads')
ax1.set_ylabel('tempo (s)')
ax1.set_title('Calculo de Pi, N=2e9 - tempo medio (30 execucoes)')
ax1.legend()
ax1.grid(True, linestyle='--', alpha=0.4)

speedup_openmp = [openmp[1] / openmp[k] for k in x]
speedup_mpi = [mpi[1] / mpi[k] for k in x]
speedup_hibrido = [hibrido[1] / hibrido[k] for k in x]

ax2.plot(x, speedup_openmp, marker='o', label='OpenMP puro')
ax2.plot(x, speedup_mpi, marker='s', label='MPI puro')
ax2.plot(x, speedup_hibrido, marker='^', label='Hibrido')
ax2.plot(x, x, linestyle='--', color='gray', label='speedup ideal (linear)')
ax2.set_xlabel('Numero de processos/threads')
ax2.set_ylabel('speedup (relativo a 1)')
ax2.set_title('Speedup')
ax2.legend()
ax2.grid(True, linestyle='--', alpha=0.4)

fig.tight_layout()
fig.savefig('curva_pi.png', dpi=150)
print('salvo em curva_pi.png')
