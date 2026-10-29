#ifndef CEX_DEMO_H
#define CEX_DEMO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(CEX_DEMO_STATIC)
#    define CEX_DEMO_API
#elif defined(_WIN32) || defined(__CYGWIN__)
#    if defined(CEX_DEMO_BUILD)
#        define CEX_DEMO_API __declspec(dllexport)
#    else
#        define CEX_DEMO_API __declspec(dllimport)
#    endif
#elif defined(__GNUC__) || defined(__clang__)
#    define CEX_DEMO_API __attribute__((visibility("default")))
#else
#    define CEX_DEMO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CEX_DEMO_ERR_OK = 0,
    CEX_DEMO_ERR_ARGUMENT = 1,
    CEX_DEMO_ERR_OVERFLOW = 2,
    CEX_DEMO_ERR_NOT_FOUND = 3,
    CEX_DEMO_ERR_MEMORY = 4,
    CEX_DEMO_ERR_OS = 5,
    CEX_DEMO_ERR_UNKNOWN = 99,
} cex_demo_error_t;

typedef struct
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t struct_size;
} cex_demo_version_t;

typedef struct
{
    int32_t x;
    int32_t y;
} cex_demo_point_t;

typedef struct
{
    uint8_t tag;
    int64_t big;
    double d;
    char name[16];
} cex_demo_padded_t;

typedef enum
{
    CEX_DEMO_MODE_A = 0,
    CEX_DEMO_MODE_B = 1,
    CEX_DEMO_MODE_C = 2,
} cex_demo_mode_t;

typedef struct cex_demo_counter cex_demo_counter_t;

typedef int32_t (*cex_demo_cb_fn)(int32_t value, void* user_data);
typedef void* (*cex_demo_malloc_fn)(size_t size);
typedef void (*cex_demo_free_fn)(void* ptr);

CEX_DEMO_API cex_demo_version_t cex_demo_version(void);
CEX_DEMO_API size_t cex_demo_sizeof_padded(void);

CEX_DEMO_API int8_t cex_demo_id_i8(int8_t v);
CEX_DEMO_API uint64_t cex_demo_id_u64(uint64_t v);
CEX_DEMO_API double cex_demo_mix(double a, float b, int8_t c, uint8_t d);
CEX_DEMO_API bool cex_demo_is_odd(uint64_t v);

CEX_DEMO_API size_t cex_demo_str_len(const char* s);
/* Caller owns `out`; returns 0 on success, -1 when `out_len` is too small. */
CEX_DEMO_API int cex_demo_str_concat(char* out, size_t out_len, const char* a, const char* b);
CEX_DEMO_API size_t cex_demo_str_concat_need(const char* a, const char* b);

CEX_DEMO_API int64_t cex_demo_sum_i32(const int32_t* arr, size_t len);
CEX_DEMO_API void cex_demo_scale_i32(int32_t* arr, size_t len, int32_t factor);

CEX_DEMO_API cex_demo_point_t cex_demo_point_add(cex_demo_point_t a, cex_demo_point_t b);
CEX_DEMO_API void cex_demo_point_scale(cex_demo_point_t* p, int32_t factor);
CEX_DEMO_API cex_demo_padded_t cex_demo_padded_make(uint8_t tag, int64_t big, double d, const char* name);

CEX_DEMO_API cex_demo_mode_t cex_demo_mode_next(cex_demo_mode_t mode);
CEX_DEMO_API int cex_demo_divmod(int32_t a, int32_t b, int32_t* out_q, int32_t* out_r);

CEX_DEMO_API cex_demo_counter_t* cex_demo_counter_new(int32_t start);
CEX_DEMO_API int32_t cex_demo_counter_next(cex_demo_counter_t* c);
CEX_DEMO_API void cex_demo_counter_free(cex_demo_counter_t* c);

/* Library-owned allocation. Free with cex_demo_free(). */
CEX_DEMO_API char* cex_demo_str_dup(const char* s);
CEX_DEMO_API void cex_demo_free(void* p);
/* Allocates with the caller's allocator; caller frees with its own free(). */
CEX_DEMO_API char* cex_demo_str_dup_with(const char* s, cex_demo_malloc_fn m, cex_demo_free_fn f);
/* Library-owned allocation. Free with cex_demo_free(), never with libc free(). */
CEX_DEMO_API char* cex_demo_hazard_lib_alloc(const char* s);
/* Takes caller-owned memory and frees it through the caller's callback. */
CEX_DEMO_API int cex_demo_hazard_take(char* p, cex_demo_free_fn f);

CEX_DEMO_API int32_t cex_demo_apply(int32_t* arr, size_t len, cex_demo_cb_fn cb, void* user_data);
CEX_DEMO_API int32_t cex_demo_call_on_thread(cex_demo_cb_fn cb, void* user_data, int32_t value);

/* Raw cex Exc: NULL means success, error strings are borrowed and never freed. */
CEX_DEMO_API const char* cex_demo_parse_i32_exc(const char* s, int32_t* out);
CEX_DEMO_API int32_t cex_demo_parse_i32_status(const char* s, int32_t* out);
CEX_DEMO_API int32_t cex_demo_parse_i32_hybrid(const char* s, int32_t* out, const char** out_exc);

CEX_DEMO_API extern const char* const cex_demo_Err_ok;
CEX_DEMO_API extern const char* const cex_demo_Err_argument;
CEX_DEMO_API extern const char* const cex_demo_Err_overflow;
CEX_DEMO_API extern const char* const cex_demo_Err_not_found;
CEX_DEMO_API extern const char* const cex_demo_Err_memory;

CEX_DEMO_API const char* cex_demo_error_string(const char* exc);
CEX_DEMO_API int32_t cex_demo_error_code(const char* exc);
CEX_DEMO_API const char* cex_demo_strerror(int32_t code);
/* Thread-local. Empty when the calling thread has no recorded error. */
CEX_DEMO_API const char* cex_demo_last_traceback(void);

#ifdef __cplusplus
}
#endif

#endif
