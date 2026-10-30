# cex_demo

Minimal CEX-built static/shared library with two exported functions.

## API

`lib/cex_demo.h`:

- `cex_demo_request_t` — one `int32_t value`.
- `cex_demo_run(cex_demo_request_t* req)` — returns `cex_demo_exc_t`
  (`const char*`), where `NULL` means success. A `NULL` request returns an error;
  otherwise `req->value` is incremented.
- `cex_demo_traceback(char* buf, size_t buf_size)` — formats the recorded CEX
  traceback into `buf` (NUL-terminated). Returns `ok` on success, `argument` for
  a `NULL`/zero-size buffer, and `overflow` when the traceback does not fit.
- `cex_demo_Error` — exported struct with `ok` (`NULL`), `argument`, and
  `overflow` pointers. Compare errors by pointer against these fields; the
  string is display-only.

## Build and test

```sh
cc cex.c -o cex
./cex test run all
./cex build-lib
./cex bindings
```

`./cex build-lib` writes `build/lib/libcex_demo.a` and the shared library.
`./cex bindings` builds the library once and runs every binding.

`lib/cex_demo.c` includes `cex.h`; `cex.h` is vendored and must not be edited.

## Bindings

Eight language bindings live under `bindings/`: `c_static`, `cpp_static`,
`go_cgo`, `nodejs_koffi`, `python_ctypes`, `python_cffi`, `rust_static`, and
`zig_static`. Each is an ergonomics example, not a test suite.

```sh
./cex bindings              # all bindings
./cex bindings c_static     # one binding
```

`./cex bindings` builds the library once, runs every binding, and summarizes
pass/fail. Each example compares `cex_demo_exc_t` by pointer against
`cex_demo_Error.ok` / `.argument`, then prints the traceback returned by
`cex_demo_traceback`. See `bindings/README.md` for the per-binding contract and
how to add a language.
