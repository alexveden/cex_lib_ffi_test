#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

OUT="build/bindings/cpp_static"
mkdir -p "$OUT"

g++ -Wall -Wextra -Werror -std=c++20 -Ilib \
    -c bindings/cpp_static/main.cpp \
    -o "$OUT/main.o"

g++ "$OUT/main.o" build/lib/libcex_demo.a -o "$OUT/main"

"$OUT/main"
