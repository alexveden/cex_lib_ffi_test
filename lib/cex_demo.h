#ifndef CEX_DEMO_H
#define CEX_DEMO_H

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

typedef const char* cex_demo_exc_t;

typedef struct
{
    cex_demo_exc_t ok;
    cex_demo_exc_t argument;
    cex_demo_exc_t overflow;
} cex_demo_errors_t;

typedef struct
{
    int32_t value;
} cex_demo_request_t;

/* NULL req -> error; otherwise increments req->value. NULL return = success. */
CEX_DEMO_API cex_demo_exc_t cex_demo_run(cex_demo_request_t* req);

/* Formats the recorded traceback into buf (NUL-terminated).
 * NULL/zero buffer -> Error.argument; traceback does not fit -> Error.overflow. */
CEX_DEMO_API cex_demo_exc_t cex_demo_traceback(char* buf, size_t buf_size);

#if defined(CEX_DEMO_PREBUILT)
CEX_DEMO_API extern cex_demo_errors_t cex_demo_Error;
#else
CEX_DEMO_API extern const cex_demo_errors_t cex_demo_Error;
#endif

#ifdef __cplusplus
}
#endif

#if !defined(CEX_DEMO_BUILD)
#    define Error cex_demo_Error
#endif

#endif
