# AGENTS.md

`cex_demo` is a minimal CEX-built FFI proof: one public header, one exported
function, one native test.

## Layout

- `lib/cex_demo.h` — public ABI (`stdint.h` only).
- `lib/cex_demo.c` — implementation (`CEX_IMPLEMENTATION` + `CEX_DEMO_BUILD`).
- `tests/test_cex_demo.c` — native tests.
- `cex.c` — CEX CLI, including the custom `build-lib` command.
- `cex.h` — vendored CEX library; never edit.

## Build and test

- `./cex` is gitignored; create it with `cc cex.c -o cex`.
- `./cex build-lib` builds `build/lib/libcex_demo.a` and the shared library.
- `./cex test run all` runs the native tests (ASan/UBSan).
- One case: `./cex test run tests/test_cex_demo.c --filter='cex_demo_run_increments'`.

## Bindings

- `bindings/<language>_<mechanism>/` — one ergonomics example per binding.
- Each dir has a `run.sh` invoked via `bash` from the repo root; it builds and
  runs the example, which prints `value=42` and `error=ArgumentError`.
- `bash run_bindings.sh` builds the library once, runs all bindings, and exits
  non-zero if any fail.
- Current: `c_static`, `python_ctypes`, `python_cffi`.
- Add a language: create the dir, mirror the ABI by hand, add `run.sh`;
  discovery is automatic.

## ABI

- One function (`cex_demo_run`) plus one data export (`cex_demo_Error`, fields
  `ok` and `argument`). Library builds use `-fvisibility=hidden`, so anything
  without `CEX_DEMO_API` stays hidden.
- `cex_demo_exc_t` is `const char*`; `NULL` = success. Compare errors by pointer
  against `cex_demo_Error.ok` / `.argument`; do not compare strings.
- `cex_demo_request_t` is a single `int32_t value`; a `NULL` request returns
  `cex_demo_Error.argument`.
- Unity builds (`tests/`) define `CEX_DEMO_PREBUILT`; the library then fills
  `cex_demo_Error` from CEX's `Error` in a constructor.
- Changing the ABI means touching `lib/cex_demo.h`, `lib/cex_demo.c`, and
  `tests/test_cex_demo.c`.
