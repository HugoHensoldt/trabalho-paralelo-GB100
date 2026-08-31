#!/bin/bash

# Compilação para offload de OpenMP em GPU NVIDIA.
# Use o compilador disponível no cluster (NVHPC/PGI têm o melhor suporte a
# "target" para NVIDIA; GCC também suporta via -foffload=nvptx-none a
# partir da versão 7+, desde que compilado com esse backend).

module load nvhpc

# -mp=gpu   -> habilita OpenMP com offload para GPU (equivalente ao antigo -ta=tesla no PGI)
# -Minfo=mp -> imprime no terminal quais regiões foram de fato geradas para o device (diagnóstico)
nvc -mp=gpu -Minfo=mp off_codigo1.c -o off_codigo1
nvc -mp=gpu -Minfo=mp off_codigo3.c -o off_codigo3

echo '*** COMPILACAO COMPLETADA ***'

# Alternativa com GCC (se o módulo NVHPC não estiver disponível):
# module load gcc/10
# gcc -fopenmp -foffload=nvptx-none off_codigo3.c -o off_codigo3
