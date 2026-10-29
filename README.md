# cex_demo — CEX static/shared FFI proof

A small C library built by CEX, exposed through a dependency-free C ABI, and
consumed from C, Python (ctypes + cffi), Node (koffi), and Zig (static + dynamic).

The point is to show which common C patterns survive the FFI boundary, and where
they break. The library itself is the proof vehicle, not the product.

## What is proven

- One plain-C header (`lib/cex_demo.h`) with no `cex.h` dependency.
- Static archive and shared library built by `./cex build-lib`.
- 38 exported `cex_demo_*` symbols, no CEX internals leaked.
- Identical behaviour across six consumers on Linux: C static, C `dlopen`,
  Python ctypes, Python cffi (static link), Node koffi, Zig static, Zig dynamic.
- Cross-built artifacts: Windows x64 DLL, macOS arm64/x64 dylibs, Linux aarch64 `.so`.
- A GitHub Actions matrix that runs the same suite on Linux, macOS, and Windows.

## Layout

```
lib/cex_demo.h              public ABI (stdint/stddef/stdbool only)
lib/cex_demo.c              implementation (CEX_IMPLEMENTATION + API)
src/myapp.c                 unity smoke app
tests/test_cex_demo.c       native cex unit tests
tests/conformance/cases.tsv shared test vectors
tests/conformance/runner_c.c        C runner, static + dlopen
tests/conformance/compare.py        result differ
tests/conformance/run_all.sh        build + run every binding
tests/conformance/check_exports.py  export-table assertions
bindings/python/ctypes_runner.py    dynamic
bindings/python/cffi_build.py       static (API mode)
bindings/python/cffi_runner.py      static
bindings/node/koffi_runner.js       dynamic
bindings/zig/runner_common.zig      shared dispatcher
bindings/zig/runner_static.zig      static link
bindings/zig/runner_dyn.zig         std.DynLib
```

## Run it

```sh
cc cex.c -o cex
./cex build-lib                 # build/lib/libcex_demo.a + .so/.dylib/.dll
./cex test run all              # native cex unit tests
bash tests/conformance/run_all.sh
```

Cross artifacts and export checks:

```sh
./cex build-lib --target=all    # native + x86_64-windows-gnu, *-macos, aarch64-linux-gnu
python3 tests/conformance/check_exports.py
```

`run_all.sh` builds the C runners, the Zig runners, a venv with cffi, and
installs koffi if needed. Set `SKIP_CFFI=1` or `SKIP_ZIG=1` to omit those.

## Capability matrix

All cells are `PASS` unless noted. `static` means the `.a`/`.lib` is linked at
build time; `dynamic` means the shared library is loaded at runtime.

| Pattern | C static | C dynamic | Py ctypes | Py cffi static | Node koffi | Zig static | Zig dynamic |
|---|---|---|---|---|---|---|---|
| fixed-width ints (i8/u64) | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| float/double/bool | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| NUL strings in/out | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| caller buffer + len | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| two-call size query | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| arrays (ptr + count) | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| struct by value | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| struct by pointer | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| padded struct (40 bytes) | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| enum | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| out-params | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| opaque handle | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| caller-owned alloc | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| library-owned + free | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| caller allocator callbacks | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| callback + user_data | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| callback from lib thread | PASS | PASS | PASS | PASS | SKIP | PASS | PASS |
| raw `Exc` + helpers | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| int32 status codes | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| hybrid code + raw Exc | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| exported error consts | PASS | PASS | PASS | PASS | PASS | PASS | PASS |
| thread-local traceback | PASS | PASS | PASS | PASS | PASS | PASS | PASS |

Node `call_on_thread` is `SKIP`: koffi queues a foreign-thread callback to the
JS main thread, but the synchronous FFI call is blocking that thread, so the
callback cannot run. This is a real limitation, not a bug in the library.

## ABI rules

- **Strings** are UTF-8 `const char*`. Borrowed strings are valid until the next
  call on the same thread; never free them.
- **Errors** are `const char*` where `NULL` is success. Compare by pointer
  identity against the exported `cex_demo_Err_*` values, or use
  `cex_demo_error_code` / `cex_demo_error_string`.
- **Ownership**: caller passes output buffers; library-owned memory is released
  only with `cex_demo_free`; caller-allocator variants free with the caller's own
  `free`.
- **Structs** use default C alignment; `cex_demo_padded_t` is 40 bytes on all
  tested ABIs. No `#pragma pack`.
- **Enums** are `int`-sized (4 bytes), asserted at build time.
- **bool** is 1 byte (`_Bool`), mapped to `bool` by Zig/koffi and `c_bool` by ctypes.
- **Tracebacks** are `_Thread_local`; an error raised on a library thread is not
  visible from the caller's thread.

## Rough edges found

- `Exc` is just `char*`; pointer identity is ABI-trivial but most high-level FFI
  layers convert `char*` to a string and lose the address. Bindings must keep
  pointer-returning functions as `void*`/`c_void_p` to compare identity.
- Python `ctypes.c_char_p` returns `bytes`, not the pointer; `c_void_p` is
  required for identity checks.
- cffi needs `extern` declarations for exported globals and explicit `malloc`/
  `free` in `cdef` before `ffi.dlopen(None)` can expose them.
- Node koffi returns 64-bit values as `Number` when they fit in 53 bits and as
  `BigInt` otherwise; both stringify identically, but arithmetic differs.
- Static linking from a managed runtime needs a native shim. cffi API mode does
  it for Python; Node would need an N-API addon (not attempted).
- Windows symbol hygiene: `__declspec(dllexport)` disables MinGW auto-export, so
  the DLL exports exactly the 38 API symbols. `--exclude-all-symbols` is not
  accepted by Zig's LLD, so it is not used.
- Mach-O prefixes exported C symbols with `_`; the export check strips it.
- The native `.so` is ~560 KB because the CEX runtime is embedded; the cross
  aarch64 `.so` is larger (~5.6 MB) due to debug info and static libc.
- `os.fs.mkpath` creates the parent directory of its argument, not the argument
  itself; `build-lib` passes a child path to create `build/lib`.

## Not covered

Tier C patterns (variadics, bitfields, unions, C++ name mangling, Windows calling
conventions), Rust/Go/Java/.NET/Perl bindings, C++ consumers, and packaging.
