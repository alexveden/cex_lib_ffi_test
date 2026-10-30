#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

command -v go >/dev/null || {
    echo "go toolchain not found" >&2
    exit 1
}

OUT="build/bindings/go_cgo"
mkdir -p "$OUT"

CGO_ENABLED=1 go build -C bindings/go_cgo -o "../../$OUT/main" .

"$OUT/main"
