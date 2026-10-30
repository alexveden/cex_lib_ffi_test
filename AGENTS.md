# AGENTS.md

`cex_demo` is a minimal CEX-built FFI proof: one public header, two exported
functions, one native test.

## Layout

- `lib/cex_demo.h` — public ABI (`stdint.h` + `stddef.h`).
- `lib/cex_demo.c` — implementation (`CEX_IMPLEMENTATION` + `CEX_DEMO_BUILD`).
- `tests/test_cex_demo.c` — native tests.
- `cex.c` — CEX CLI, including the custom `build-lib` and `bindings` commands.
- `cex.h` — vendored CEX library; never edit.

## Build and test

- `./cex` is gitignored; create it with `cc cex.c -o cex`.
- `./cex build-lib` builds `build/lib/libcex_demo.a` and the shared library.
- `./cex test run all` runs the native tests (ASan/UBSan).
- One case: `./cex test run tests/test_cex_demo.c --filter='cex_demo_run_increments'`.
- `./cex bindings [all|<name>...]` builds the library once and runs bindings.

## Bindings

- `bindings/<language>_<mechanism>/` — one ergonomics example per binding.
- Orchestration lives in `cex.c`: each binding has a `bindings__run_*` runner
  and an entry in `bindings__table`; no per-binding scripts.
- `./cex bindings` or `./cex bindings all` runs every binding, prints
  `value=42`, `error=ArgumentError`, and a `traceback=` block per example, and
  exits non-zero if any fail. `./cex bindings <name>...` runs only the named
  bindings.
- Current: `c_static`, `cpp_static`, `go_cgo`, `nodejs_koffi`,
  `python_ctypes`, `python_cffi`, `rust_static`, `zig_static`.
- Add a language: create the dir, mirror the ABI by hand, add a runner and a
  table entry in `cex.c`.

## ABI

- Two functions (`cex_demo_run`, `cex_demo_traceback`) plus one data export
  (`cex_demo_Error`, fields `ok`, `argument`, `overflow`). Library builds use
  `-fvisibility=hidden`, so anything without `CEX_DEMO_API` stays hidden.
- `cex_demo_exc_t` is `const char*`; `NULL` = success. Compare errors by pointer
  against `cex_demo_Error.ok` / `.argument` / `.overflow`; do not compare strings.
- `cex_demo_request_t` is a single `int32_t value`; a `NULL` request returns
  `cex_demo_Error.argument`.
- `cex_demo_traceback(char* buf, size_t buf_size)` formats the recorded CEX
  traceback into `buf`; `NULL`/zero buffer returns `argument`, insufficient
  space returns `overflow`. It uses `cexsp__snprintf` directly so the traceback
  ring is not reset on overflow.
- Unity builds (`tests/`) define `CEX_DEMO_PREBUILT`; the library then fills
  `cex_demo_Error` from CEX's `Error` in a constructor.
- Changing the ABI means touching `lib/cex_demo.h`, `lib/cex_demo.c`, and
  `tests/test_cex_demo.c`.
