#!/bin/bash

# Compilação da versão CUDA de off_simples (soma de vetores na GPU).

module load cuda

# nvcc é o compilador CUDA (incluído no módulo cuda do SDumont)
nvcc off_simples.cu -o off_simples

echo '*** COMPILACAO COMPLETADA ***'
