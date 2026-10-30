#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

command -v node >/dev/null || {
    echo "node not found" >&2
    exit 1
}

NODE_MODULES="$ROOT/build/node/node_modules"
if [ ! -d "$NODE_MODULES/koffi" ]; then
    npm install --prefix build/node --no-audit --no-fund --silent koffi
fi

NODE_PATH="$NODE_MODULES" node bindings/nodejs_koffi/main.js
