#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

CASES="tests/conformance/cases.tsv"
OUT="build/conformance"
STATUS=0

case "$(uname -s)" in
    Linux*)
        STAT_EXT=".a"; DYN_EXT=".so"; DL_LIB="-ldl"; THREAD_FLAG="-pthread"
        ;;
    Darwin*)
        STAT_EXT=".a"; DYN_EXT=".dylib"; DL_LIB=""; THREAD_FLAG="-pthread"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        STAT_EXT=".lib"; DYN_EXT=".dll"; DL_LIB=""; THREAD_FLAG=""
        ;;
    *)
        STAT_EXT=".a"; DYN_EXT=".so"; DL_LIB=""; THREAD_FLAG=""
        ;;
esac

STAT_LIB="build/lib/libcex_demo$STAT_EXT"
DYN_LIB="build/lib/libcex_demo$DYN_EXT"

PY_BOOT="$(command -v python3 || command -v python || true)"
if [ -z "$PY_BOOT" ]; then
    echo "python not found"
    exit 1
fi

mkdir -p "$OUT"

echo "== build library =="
./cex build-lib || exit 1

echo "== build C runners =="
cc -DCEX_DEMO_STATIC -Wall -Wextra -Werror -Ilib \
    tests/conformance/runner_c.c "$STAT_LIB" $THREAD_FLAG \
    -o "$OUT/runner_c_static" || exit 1

cc -DCEX_DEMO_DYNAMIC -Wall -Wextra -Werror -Ilib \
    tests/conformance/runner_c.c $DL_LIB \
    -o "$OUT/runner_c_dynamic" || exit 1

echo "== build Zig runners =="
if [ "${SKIP_ZIG:-0}" = "1" ]; then
    echo "SKIP_ZIG=1: skipping Zig runners"
elif command -v zig >/dev/null 2>&1; then
    zig build-exe bindings/zig/runner_static.zig "$STAT_LIB" -lc \
        -femit-bin="$OUT/runner_zig_static" || exit 1
    zig build-exe bindings/zig/runner_dyn.zig -lc \
        -femit-bin="$OUT/runner_zig_dyn" || exit 1
else
    echo "zig not found: skipping Zig runners"
fi

echo "== build Python cffi module =="
if [ "${SKIP_CFFI:-0}" = "1" ]; then
    echo "SKIP_CFFI=1: skipping cffi build"
    PY="$PY_BOOT"
else
    if [ ! -x build/venv/bin/python ] && [ ! -x build/venv/Scripts/python.exe ]; then
        "$PY_BOOT" -m venv build/venv
        build/venv/bin/pip install --quiet cffi setuptools 2>/dev/null \
            || build/venv/Scripts/pip install --quiet cffi setuptools
    fi
    PY="build/venv/bin/python"
    if [ ! -x "$PY" ]; then PY="build/venv/Scripts/python.exe"; fi
    "$PY" bindings/python/cffi_build.py >/dev/null || exit 1
fi

echo "== ensure Node koffi =="
if [ ! -d bindings/node/node_modules/koffi ]; then
    (cd bindings/node && npm install --silent)
fi

run_runner() {
    local name="$1"
    shift
    local out="$OUT/$name.out"
    local err="$OUT/$name.err"
    if "$@" >"$out" 2>"$err"; then
        :
    else
        echo "RUNNER $name exited non-zero (see $err)"
    fi
    "$PY" tests/conformance/compare.py "$CASES" "$out" "$name" || STATUS=1
}

echo "== conformance =="
run_runner c_static "$OUT/runner_c_static" "$CASES"
run_runner c_dynamic "$OUT/runner_c_dynamic" "$CASES" "$DYN_LIB"
run_runner py_ctypes "$PY" bindings/python/ctypes_runner.py "$CASES" "$DYN_LIB"
if [ "${SKIP_CFFI:-0}" = "1" ]; then
    echo "SUMMARY py_cffi skipped=all (SKIP_CFFI=1)"
else
    run_runner py_cffi "$PY" bindings/python/cffi_runner.py "$CASES" ignored
fi

if command -v node >/dev/null 2>&1; then
    run_runner node_koffi node bindings/node/koffi_runner.js "$CASES" "$DYN_LIB"
else
    echo "SUMMARY node_koffi skipped=all (node not found)"
fi

if [ -x "$OUT/runner_zig_static" ] || [ -x "$OUT/runner_zig_static.exe" ]; then
    run_runner zig_static "$OUT/runner_zig_static" "$CASES"
    run_runner zig_dynamic "$OUT/runner_zig_dyn" "$CASES" "$DYN_LIB"
else
    echo "SUMMARY zig_static skipped=all (zig not found)"
    echo "SUMMARY zig_dynamic skipped=all (zig not found)"
fi

exit "$STATUS"
