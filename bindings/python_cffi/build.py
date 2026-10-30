import os

from cffi import FFI

ROOT = os.getcwd()
BUILD = os.path.join(ROOT, "build", "cffi")

ffi = FFI()
ffi.cdef(
    """
typedef struct { int32_t value; } cex_demo_request_t;
typedef const char *cex_demo_exc_t;
typedef struct { const char *ok; const char *argument; const char *overflow; } cex_demo_errors_t;
extern const cex_demo_errors_t cex_demo_Error;
cex_demo_exc_t cex_demo_run(cex_demo_request_t *req);
cex_demo_exc_t cex_demo_traceback(char *buf, size_t size);
"""
)

if os.name == "nt":
    STATIC_LIB = os.path.join(ROOT, "build", "lib", "libcex_demo.lib")
    LINK_ARGS = [STATIC_LIB]
else:
    STATIC_LIB = os.path.join(ROOT, "build", "lib", "libcex_demo.a")
    LINK_ARGS = [STATIC_LIB]

ffi.set_source(
    "_cex_demo_cffi",
    '#include "cex_demo.h"',
    include_dirs=[os.path.join(ROOT, "lib")],
    extra_compile_args=["-DCEX_DEMO_STATIC"],
    extra_link_args=LINK_ARGS,
)

if __name__ == "__main__":
    os.makedirs(BUILD, exist_ok=True)
    os.chdir(BUILD)
    ffi.compile(verbose=False)
