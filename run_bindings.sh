#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

./cex build-lib || exit 1

status=0
passed=0
failed=0
for script in bindings/*/run.sh; do
    [ -f "$script" ] || continue
    name="$(basename "$(dirname "$script")")"
    echo "== $name =="
    if bash "$script"; then
        echo "PASS $name"
        passed=$((passed + 1))
    else
        echo "FAIL $name"
        failed=$((failed + 1))
        status=1
    fi
done

echo "bindings: passed=$passed failed=$failed"
exit "$status"
