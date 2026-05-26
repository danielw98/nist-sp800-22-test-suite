#!/usr/bin/env bash
# Compileaza, ruleaza testele unitare si testul spectral pe toate fisierele NIST.
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

echo; echo "=== teste unitare (doctest) ==="; echo
./build/dft_tests.exe

echo; echo "=== assessment peste fisierele de date NIST ==="; echo
./build/nist_assess.exe -t dft data/data.e data/data.pi data/data.sqrt2 data/data.sqrt3 data/data.sha1 |
    tee tests/results.txt
