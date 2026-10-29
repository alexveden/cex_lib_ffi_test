import os
import sys

from cffi import FFI

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
BUILD = os.path.join(ROOT, "build", "cffi")

ffi = FFI()
ffi.cdef(
    """
typedef struct { uint32_t major; uint32_t minor; uint32_t patch; uint32_t struct_size; } cex_demo_version_t;
typedef struct { int32_t x; int32_t y; } cex_demo_point_t;
typedef struct { uint8_t tag; int64_t big; double d; char name[16]; } cex_demo_padded_t;
typedef struct { const char *ok; const char *argument; const char *overflow; } cex_demo_errors_t;
typedef enum { CEX_DEMO_MODE_A=0, CEX_DEMO_MODE_B=1, CEX_DEMO_MODE_C=2 } cex_demo_mode_t;
typedef struct cex_demo_counter cex_demo_counter_t;
typedef int32_t (*cex_demo_cb_fn)(int32_t value, void *user_data);
typedef void *(*cex_demo_malloc_fn)(size_t size);
typedef void (*cex_demo_free_fn)(void *ptr);
void *malloc(size_t size);
void free(void *ptr);

cex_demo_version_t cex_demo_version(void);
size_t cex_demo_sizeof_padded(void);
int8_t cex_demo_id_i8(int8_t v);
uint64_t cex_demo_id_u64(uint64_t v);
double cex_demo_mix(double a, float b, int8_t c, uint8_t d);
_Bool cex_demo_is_odd(uint64_t v);
size_t cex_demo_str_len(const char *s);
int cex_demo_str_concat(char *out, size_t out_len, const char *a, const char *b);
size_t cex_demo_str_concat_need(const char *a, const char *b);
int64_t cex_demo_sum_i32(const int32_t *arr, size_t len);
void cex_demo_scale_i32(int32_t *arr, size_t len, int32_t factor);
cex_demo_point_t cex_demo_point_add(cex_demo_point_t a, cex_demo_point_t b);
void cex_demo_point_scale(cex_demo_point_t *p, int32_t factor);
cex_demo_padded_t cex_demo_padded_make(uint8_t tag, int64_t big, double d, const char *name);
cex_demo_mode_t cex_demo_mode_next(cex_demo_mode_t mode);
int cex_demo_divmod(int32_t a, int32_t b, int32_t *out_q, int32_t *out_r);
cex_demo_counter_t *cex_demo_counter_new(int32_t start);
int32_t cex_demo_counter_next(cex_demo_counter_t *c);
void cex_demo_counter_free(cex_demo_counter_t *c);
char *cex_demo_str_dup(const char *s);
void cex_demo_free(void *p);
char *cex_demo_str_dup_with(const char *s, cex_demo_malloc_fn m, cex_demo_free_fn f);
char *cex_demo_hazard_lib_alloc(const char *s);
int cex_demo_hazard_take(char *p, cex_demo_free_fn f);
int32_t cex_demo_apply(int32_t *arr, size_t len, cex_demo_cb_fn cb, void *user_data);
int32_t cex_demo_call_on_thread(cex_demo_cb_fn cb, void *user_data, int32_t value);
const char *cex_demo_parse_i32_exc(const char *s, int32_t *out);
int32_t cex_demo_parse_i32_status(const char *s, int32_t *out);
int32_t cex_demo_parse_i32_hybrid(const char *s, int32_t *out, const char **out_exc);
extern const cex_demo_errors_t cex_demo_Error;
const char *cex_demo_error_string(const char *exc);
int32_t cex_demo_error_code(const char *exc);
const char *cex_demo_strerror(int32_t code);
const char *cex_demo_last_traceback(void);
"""
)

if os.name == "nt":
    STATIC_LIB = os.path.join(ROOT, "build", "lib", "libcex_demo.lib")
    LINK_ARGS = [STATIC_LIB]
else:
    STATIC_LIB = os.path.join(ROOT, "build", "lib", "libcex_demo.a")
    LINK_ARGS = [STATIC_LIB, "-pthread"]

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
    print("built _cex_demo_cffi in", BUILD)
