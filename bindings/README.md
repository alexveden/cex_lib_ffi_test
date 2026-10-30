# Bindings

Each subdirectory is one language binding of `lib/cex_demo.h`. These are
ergonomics examples, not a test suite.

## Contract

- Directory name: `<language>_<mechanism>` (snake_case), e.g. `c_static`,
  `python_ctypes`, `python_cffi`.
- Contains only the example source. `./cex bindings <name>` builds the library,
  builds the example if needed, and runs it.
- The example prints:
  ```
  value=42
  error=ArgumentError
  traceback=#0 (cex_demo.c:NN cex_demo__validate_request()) [ArgumentError] request is NULL
  #1 (cex_demo.c:NN cex_demo_run()) [ArgumentError] cex_demo__validate_request(req)
  ```
  and exits non-zero if any assertion fails. The traceback is asserted by
  substring (`cex_demo__validate_request`, `cex_demo_run`), not exact line
  numbers.
- Assertions compare `cex_demo_exc_t` by pointer against `cex_demo_Error.ok`,
  `cex_demo_Error.argument`, and `cex_demo_Error.overflow`. The string is
  printed for display only.
- Static bindings link the platform static library under `build/lib/`. Dynamic
  bindings load the platform shared library from `build/lib/`.
- The example shows both the raw FFI call and a thin, idiomatic wrapper.
- cffi may install its dependency into `build/venv`, and koffi into
  `build/node`, on first run.

## Add a language

1. `mkdir bindings/<language>_<mechanism>`.
2. Add the example source.
3. Mirror `cex_demo_request_t` and `cex_demo_run` by hand from `lib/cex_demo.h`.
4. Add a `bindings__run_*` runner and a `bindings__table` entry in `cex.c`.
5. Run `./cex bindings <language>_<mechanism>`.

## Current bindings

- `c_static` — C consumer linked against the static archive.
- `cpp_static` — C++ consumer, `std::optional`/exception wrapper.
- `go_cgo` — cgo, static archive via `#cgo LDFLAGS`.
- `nodejs_koffi` — koffi on the shared library.
- `python_ctypes` — `ctypes.CDLL` on the shared library.
- `python_cffi` — cffi API mode, compiled extension linked against the archive.
- `rust_static` — Cargo crate, `build.rs` links the archive.
- `zig_static` — Zig `@cImport` plus `zig build-exe` linking the archive.

## Notes

- `go_cgo`, `rust_static` and `zig_static` report `FAIL <name>` with
  `toolchain not found` when `go`, `cargo` or `zig` is missing; `./cex bindings`
  continues with the remaining bindings and exits non-zero. CI installs all three.
- `nodejs_koffi` installs `koffi` into `build/node` from npm on first run.
  koffi cannot read exported data symbols, so the example captures
  `cex_demo_run(NULL)` — the same address as `cex_demo_Error.argument` — and
  treats `cex_demo_Error.ok` as `NULL`.
