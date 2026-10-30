#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

command -v zig >/dev/null || {
    echo "zig toolchain not found" >&2
    exit 1
}

OUT="build/bindings/zig_static"
mkdir -p "$OUT"

zig build-exe bindings/zig_static/main.zig \
    -lc -Ilib build/lib/libcex_demo.a \
    -femit-bin="$OUT/main"

"$OUT/main"
