# Bindings

Each subdirectory is one language binding of `lib/cex_demo.h`. These are
ergonomics examples, not a test suite.

## Contract

- Directory name: `<language>_<mechanism>` (snake_case), e.g. `c_static`,
  `python_ctypes`, `python_cffi`.
- Contains a `run.sh`, invoked with `bash` and the repo root as cwd. It builds
  if needed and runs the example.
- The example prints exactly:
  ```
  value=42
  error=ArgumentError
  ```
  and exits non-zero if either assertion fails.
- Assertions compare `cex_demo_exc_t` by pointer against `cex_demo_Error.ok` and
  `cex_demo_Error.argument`. The string is printed for display only.
- Static bindings link `build/lib/libcex_demo.a`. Dynamic bindings load the
  platform shared library from `build/lib/`.
- The example shows both the raw FFI call and a thin, idiomatic wrapper.
- No arguments to `run.sh`. cffi may install its dependency into `build/venv`,
  and koffi into `build/node`, on first run.

## Add a language

1. `mkdir bindings/<language>_<mechanism>`.
2. Add the example source and a `run.sh` that `cd`s to the repo root.
3. Mirror `cex_demo_request_t` and `cex_demo_run` by hand from `lib/cex_demo.h`.
4. Run `bash run_bindings.sh`; the new directory is discovered automatically.

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

- `go_cgo`, `rust_static` and `zig_static` exit 1 with `toolchain not found`
  when `go`, `cargo` or `zig` is missing; CI installs all three.
- `nodejs_koffi` installs `koffi` into `build/node` from npm on first run.
  koffi cannot read exported data symbols, so the example captures
  `cex_demo_run(NULL)` — the same address as `cex_demo_Error.argument` — and
  treats `cex_demo_Error.ok` as `NULL`.
