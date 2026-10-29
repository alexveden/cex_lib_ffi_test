#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

PY="build/venv/bin/python"
[ -x "$PY" ] || PY="build/venv/Scripts/python.exe"

if [ ! -x "$PY" ]; then
    python3 -m venv build/venv
    PY="build/venv/bin/python"
    [ -x "$PY" ] || PY="build/venv/Scripts/python.exe"
fi

"$PY" -c "import cffi" >/dev/null 2>&1 || "$PY" -m pip install --quiet cffi setuptools

"$PY" bindings/python_cffi/build.py
"$PY" bindings/python_cffi/main.py
