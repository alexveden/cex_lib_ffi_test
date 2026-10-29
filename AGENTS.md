# AGENTS.md

`cex_demo` is an FFI/ABI conformance proof, not an application. `lib/cex_demo.h` is a
dependency-free public C ABI; C, Python, Node, and Zig consumers must all produce
identical output for the vectors in `tests/conformance/cases.tsv`. Keep the ABI stable
and the consumers in sync.

## Bootstrap and build

- `./cex` is gitignored; create it with `cc cex.c -o cex`. It self-rebuilds after
  `cex.c`/`cex.h` change, so rebuild before debugging odd CLI behavior.
- `./cex build-lib` builds `build/lib/libcex_demo.a` + `.so`/`.dylib`/`.dll`.
- `./cex build-lib --target=all` adds the Windows/macOS/aarch64 cross builds; requires `zig`.
- Do not scaffold over `cex.c`: it is hand-customized with the `build-lib` command and
  `cexy$cc_include` config, which `./cex new` boilerplate does not contain.
- Never edit `cex.h`; it is the vendored 22k-line CEX library.

## Tests

- Native unit tests: `./cex test run all` (built with ASan/UBSan, so leaks/UAF fail the run).
- One case: `./cex test run tests/test_cex_demo.c --filter='cex_demo_primitives'`.
- Full FFI matrix: `bash tests/conformance/run_all.sh` — builds the lib, C static/dlopen,
  Zig static/dynamic, cffi venv, and koffi, then diffs every runner against `cases.tsv`.
  `SKIP_CFFI=1` / `SKIP_ZIG=1` skip optional consumers. This is separate from `./cex test`.
- Export table: `./cex build-lib --target=all && python3 tests/conformance/check_exports.py`
  asserts exactly 34 `cex_demo_*` exports and zero leaked symbols; needs `llvm-nm` /
  `llvm-readobj` (`llvm` package).
- `build/venv` (cffi) and `bindings/node/node_modules` (koffi) are created on demand and
  gitignored; deleting `build/` forces a network reinstall.

## Changing the ABI

Touch every layer or the matrix breaks:

1. `lib/cex_demo.h` — stays cex-free (`stdint.h`/`stddef.h`/`stdbool.h` only); mark each
   export `CEX_DEMO_API`.
2. `lib/cex_demo.c` — implementation (`CEX_IMPLEMENTATION` + `CEX_DEMO_BUILD`).
3. `tests/test_cex_demo.c` — native coverage.
4. `tests/conformance/cases.tsv` — `id<TAB>function<TAB>args<TAB>expected`; the 4th column
   is the expected runner output, and every runner must emit `id<TAB>value`
   (`SKIP:`/`FAIL:` prefixes are understood by `compare.py`).
5. Runners: `runner_c.c`, `ctypes_runner.py`, `cffi_build.py` cdef + `cffi_runner.py`,
   `koffi_runner.js`, `runner_static.zig` + `runner_dyn.zig`.
6. Bump `EXPECTED` in `tests/conformance/check_exports.py` if the symbol count changes.
7. Keep `README.md`'s capability matrix and "rough edges" list accurate.

## Gotchas

- Unity build: `src/myapp.c` and `tests/test_cex_demo.c` `#include "lib/cex_demo.c"`;
  there is no separate link step for them. Both define `CEX_DEMO_PREBUILT` first, so
  `cex_demo_Error` is filled by a constructor from the prebuilt `Error`.
- Library builds use `-fvisibility=hidden`; anything not `CEX_DEMO_API` / not prefixed
  `cex_demo_` fails the export check.
- `Exc` is `const char*` with `NULL` = success and pointer identity as the contract;
  compare against `cex_demo_Error` (`ok`/`argument`/`overflow`) or use the helpers.
  Bindings must keep it as `void*`/`c_void_p`, never convert it to a string.
- `cex_demo_padded_t` is 40 bytes and enums are int-sized; both are `_Static_assert`ed.
- CI (`.github/workflows/ci.yml`): Linux runs the full matrix + cross + export check,
  macOS the full matrix, Windows only `SKIP_CFFI=1 SKIP_ZIG=1` under MSYS2. Toolchain:
  Zig 0.16, Node 20, Python 3.13.
- `build/` and `compile_flags.txt` are generated; do not hand-edit.
