# cex_demo

Minimal CEX-built static/shared library with one exported function.

## API

`lib/cex_demo.h`:

- `cex_demo_request_t` — one `int32_t value`.
- `cex_demo_run(cex_demo_request_t* req)` — returns `cex_demo_exc_t`
  (`const char*`), where `NULL` means success. A `NULL` request returns an error;
  otherwise `req->value` is incremented.

## Build and test

```sh
cc cex.c -o cex
./cex test run all
./cex build-lib
```

`./cex build-lib` writes `build/lib/libcex_demo.a` and the shared library.

`lib/cex_demo.c` includes `cex.h`; `cex.h` is vendored and must not be edited.
