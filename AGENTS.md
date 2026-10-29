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

## ABI

- Exactly one export: `cex_demo_run`. Library builds use `-fvisibility=hidden`,
  so anything without `CEX_DEMO_API` stays hidden.
- `cex_demo_exc_t` is `const char*`; `NULL` = success. Compare against
  `Error.argument` (from `cex.h`) or check for non-`NULL`.
- `cex_demo_request_t` is a single `int32_t value`; a `NULL` request returns
  `Error.argument`.
- Changing the ABI means touching `lib/cex_demo.h`, `lib/cex_demo.c`, and
  `tests/test_cex_demo.c`.
