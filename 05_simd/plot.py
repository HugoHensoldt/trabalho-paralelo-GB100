import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Medias das 3 rodadas de bench.sh (ver resultados.md)
n           = [100_000, 1_000_000, 5_000_000, 10_000_000, 50_000_000, 100_000_000]
tempo_on    = [0.0010, 0.0087, 0.0390, 0.0745, 0.3634, 0.7440]  # com #pragma omp simd
tempo_off   = [0.0019, 0.0167, 0.0874, 0.1595, 0.7353, 1.4404]  # sem #pragma omp simd

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))

ax1.plot(n, tempo_on, marker='o', label='com #pragma omp simd')
ax1.plot(n, tempo_off, marker='s', label='sem #pragma omp simd')
ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('N (elementos)')
ax1.set_ylabel('tempo (s)')
ax1.set_title('Tempo de execucao')
ax1.legend()
ax1.grid(True, which='both', linestyle='--', alpha=0.4)

speedup = [o/c for o, c in zip(tempo_off, tempo_on)]
ax2.plot(n, speedup, marker='o', color='tab:green')
ax2.axhline(2.0, linestyle='--', color='gray', label='2x teorico (SSE, 128 bits)')
ax2.set_xscale('log')
ax2.set_xlabel('N (elementos)')
ax2.set_ylabel('speedup (sem simd / com simd)')
ax2.set_title('Ganho do #pragma omp simd')
ax2.set_ylim(0, 2.5)
ax2.legend()
ax2.grid(True, which='both', linestyle='--', alpha=0.4)

fig.tight_layout()
fig.savefig('curva_desempenho.png', dpi=150)
print('salvo em curva_desempenho.png')
