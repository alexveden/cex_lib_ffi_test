#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

OUT="build/bindings/c_static"
mkdir -p "$OUT"

cc -Wall -Wextra -Werror -Ilib \
    -c bindings/c_static/main.c \
    -o "$OUT/main.o"

cc "$OUT/main.o" build/lib/libcex_demo.a -o "$OUT/main"

"$OUT/main"
