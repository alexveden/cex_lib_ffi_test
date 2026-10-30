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
- Assertions compare `cex_demo_exc_t` by pointer against `cex_demo_Error.ok`
  and `cex_demo_Error.argument`. The string is printed for display only.
  `python_capi`, `python_cython` and `lua_capi` are the exception: they hide the
  pointer in C and assert the error by exception class instead.
- Static bindings link the platform static library under `build/lib/`
  (`libcex_demo.a`, or `libcex_demo.lib` on Windows) and define
  `CEX_DEMO_STATIC`. Dynamic bindings load the platform shared library from
  `build/lib/`.
- The example shows one happy-path FFI call, the `NULL`-request error path, and
  the traceback.
- cffi and cython may install their dependencies into `build/venv`, and koffi
  into `build/node`, on first run.

## Add a language

1. `mkdir bindings/<language>_<mechanism>`.
2. Add the example source.
3. Mirror `cex_demo_request_t`, `cex_demo_run`, and `cex_demo_traceback` by
   hand from `lib/cex_demo.h`.
4. Add a `bindings__run_*` runner and a `bindings__table` entry in `cex.c`.
5. Run `./cex bindings <language>_<mechanism>`.

## Current bindings

- `c_static` — C consumer linked against the static archive.
- `cpp_static` — C++ consumer linked against the static archive.
- `go_cgo` — cgo, static archive via `#cgo LDFLAGS`.
- `lua_capi` — Lua C module, `Request` userdata and error tables, linked
  against the archive.
- `lua_ffi` — LuaJIT FFI on the shared library.
- `nodejs_koffi` — koffi on the shared library.
- `python_capi` — CPython C extension, `Request` type and exception classes.
- `python_ctypes` — `ctypes.CDLL` on the shared library.
- `python_cffi` — cffi API mode, compiled extension linked against the archive.
- `python_cython` — Cython `cdef extern` module built by pyximport.
- `rust_static` — Cargo crate, `build.rs` links the archive.
- `zig_static` — Zig `@cImport` plus `zig build-exe` linking the archive.

## Notes

- CI runs every binding on Linux, macOS and Windows; each job installs the
  `go`, `cargo`, `zig`, `luajit`, `node`/`npm` and Python toolchains.
- `go_cgo`, `rust_static`, `zig_static`, `lua_capi` and `lua_ffi` report
  `FAIL <name>` with `toolchain not found` when `go`, `cargo`, `zig` or `luajit`
  is missing; `./cex bindings` continues with the remaining bindings and exits
  non-zero.
- `lua_ffi` needs only the `luajit` interpreter. `lua_capi` also needs the
  LuaJIT headers and `luajit.pc`; the runner asks `pkg-config` (or `pkgconf`)
  for the include flags, links the archive into `build/bindings/lua_capi/`,
  and loads the module with `require`.
- `nodejs_koffi` installs `koffi` into `build/node` from npm on first run.
  koffi cannot read exported data symbols, so the example captures
  `cex_demo_run(NULL)` — the same address as `cex_demo_Error.argument` — and
  treats `cex_demo_Error.ok` as `NULL`.
- `python_capi` needs the Python development headers and requires CPython 3.10+
  for `PyModule_AddObjectRef`. It queries `sysconfig` for the include dir and
  extension suffix, compiles with `cc`, and has no Python package dependencies.
- `python_cython` compiles through `pyximport` on import; the runner installs
  `cython` into `build/venv` on first run and links the archive via
  `_cex_demo_cython.pyxbld`.
