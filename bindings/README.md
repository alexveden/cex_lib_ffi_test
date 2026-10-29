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
- No arguments to `run.sh`. cffi may install its dependency into `build/venv` on
  first run.

## Add a language

1. `mkdir bindings/<language>_<mechanism>`.
2. Add the example source and a `run.sh` that `cd`s to the repo root.
3. Mirror `cex_demo_request_t` and `cex_demo_run` by hand from `lib/cex_demo.h`.
4. Run `bash run_bindings.sh`; the new directory is discovered automatically.

## Current bindings

- `c_static` — C consumer linked against the static archive.
- `python_ctypes` — `ctypes.CDLL` on the shared library.
- `python_cffi` — cffi API mode, compiled extension linked against the archive.
