#!/bin/bash
# uso: ./run.sh <n>
N=${1:-25}

echo "===== fib_task (n=$N) ====="
./fib_task $N
