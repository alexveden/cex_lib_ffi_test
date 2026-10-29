#ifndef CEX_DEMO_H
#define CEX_DEMO_H

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
    int32_t value;
} cex_demo_request_t;

/* NULL req -> error; otherwise increments req->value. NULL return = success. */
CEX_DEMO_API cex_demo_exc_t cex_demo_run(cex_demo_request_t* req);

#ifdef __cplusplus
}
#endif

#endif
