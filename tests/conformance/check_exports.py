import glob
import os
import shutil
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXPECTED = 38


def find_tool(base):
    if shutil.which(base):
        return base
    for candidate in sorted(glob.glob(f"/usr/bin/{base}-*"), reverse=True):
        if os.access(candidate, os.X_OK):
            return candidate
    raise FileNotFoundError(base)


def symbols(path):
    if path.endswith(".dll"):
        out = subprocess.check_output(
            [find_tool("llvm-readobj"), "--coff-exports", path], text=True
        )
        return [
            line.split("Name: ", 1)[1].strip()
            for line in out.splitlines()
            if "Name: " in line
        ]
    if path.endswith(".dylib"):
        out = subprocess.check_output([find_tool("llvm-nm"), "-gU", path], text=True)
        result = []
        for line in out.splitlines():
            parts = line.split()
            if not parts:
                continue
            sym = parts[-1]
            if sym in ("___dso_handle", "__mh_dylib_header"):
                continue
            if sym.startswith("_") and not sym.startswith("__"):
                sym = sym[1:]
            result.append(sym)
        return result
    out = subprocess.check_output(
        [find_tool("llvm-nm"), "-D", "--defined-only", path], text=True
    )
    return [line.split()[-1] for line in out.splitlines() if line.strip()]


def check(path):
    if not os.path.exists(path):
        print(f"FAIL {path}: missing")
        return False
    syms = symbols(path)
    leaked = [s for s in syms if not s.startswith("cex_demo_")]
    if leaked:
        print(f"FAIL {path}: leaked symbols {leaked[:5]}")
        return False
    if len(syms) != EXPECTED:
        print(f"FAIL {path}: expected {EXPECTED} exports, got {len(syms)}")
        return False
    print(f"PASS {path}: {len(syms)} exports")
    return True


def main():
    if os.name == "nt":
        native = os.path.join(ROOT, "build", "lib", "libcex_demo.dll")
    elif sys.platform == "darwin":
        native = os.path.join(ROOT, "build", "lib", "libcex_demo.dylib")
    else:
        native = os.path.join(ROOT, "build", "lib", "libcex_demo.so")

    targets = [
        native,
        os.path.join(ROOT, "build", "lib", "x86_64-windows-gnu", "libcex_demo.dll"),
        os.path.join(ROOT, "build", "lib", "aarch64-macos", "libcex_demo.dylib"),
        os.path.join(ROOT, "build", "lib", "x86_64-macos", "libcex_demo.dylib"),
        os.path.join(ROOT, "build", "lib", "aarch64-linux-gnu", "libcex_demo.so"),
    ]
    ok = True
    for path in targets:
        if not check(path):
            ok = False
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
