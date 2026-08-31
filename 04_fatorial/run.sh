#!/bin/bash
echo "===== critical ====="
./fatorial_critical

echo "===== local (thread-private + atomic) ====="
./fatorial_local

echo "===== reduction ====="
./fatorial_reduction
