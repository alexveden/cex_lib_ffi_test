#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

command -v cargo >/dev/null || {
    echo "cargo toolchain not found" >&2
    exit 1
}

mkdir -p build/bindings/rust_static

CARGO_TARGET_DIR="$ROOT/build/rust" \
    cargo build --quiet --offline \
        --manifest-path bindings/rust_static/Cargo.toml

"$ROOT/build/rust/debug/cex_demo_rust_static"
