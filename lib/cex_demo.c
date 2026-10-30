#define CEX_IMPLEMENTATION
#include "cex.h"
#include <limits.h>

#define CEX_DEMO_BUILD
#include "cex_demo.h"

#if defined(CEX_DEMO_PREBUILT)
CEX_DEMO_API cex_demo_errors_t cex_demo_Error;

__attribute__((constructor)) static void
cex_demo__init_errors(void)
{
    cex_demo_Error = (cex_demo_errors_t){
        .ok = Error.ok,
        .argument = Error.argument,
        .overflow = Error.overflow,
    };
}
#else
CEX_DEMO_API const cex_demo_errors_t cex_demo_Error = {
    .ok = Error.ok,
    .argument = Error.argument,
    .overflow = Error.overflow,
};
#endif

static Exc
cex_demo__validate_request(cex_demo_request_t* req)
{
    if (!req) { return e$raise(Error.argument, "request is NULL"); }
    return EOK;
}

cex_demo_exc_t
cex_demo_run(cex_demo_request_t* req)
{
    e$ret(cex_demo__validate_request(req));
    req->value++;
    return cex_demo_Error.ok;
}

cex_demo_exc_t
cex_demo_traceback(char* buf, size_t buf_size)
{
    if (!buf || buf_size == 0) { return cex_demo_Error.argument; }
    buf[0] = '\0';
    if (buf_size > (size_t)INT_MAX) { buf_size = (size_t)INT_MAX; }

#if CEX_TRACEBACK_VERBOSITY >= 1 && CEX_TRACEBACK_VERBOSITY <= 2
    usize used = 0;
    u32 len = e$traceback_len;
    for (u32 i = 0; i < len; i++) {
        const _cex_errors_traceback_s* f = &e$traceback_arr[i];
#    if CEX_TRACEBACK_VERBOSITY == 1
        int n = cexsp__snprintf(
            buf + used,
            (int)(buf_size - used),
            "#%u (%s:%u) [%s]\n",
            i,
            f->file,
            f->line,
            f->err
        );
#    else
        int n = cexsp__snprintf(
            buf + used,
            (int)(buf_size - used),
            "#%u (%s:%u %s()) [%s] %s\n",
            i,
            f->file,
            f->line,
            f->func,
            f->err,
            f->msg
        );
#    endif
        if (n < 0 || (usize)n >= buf_size - used) {
            buf[0] = '\0';
            return cex_demo_Error.overflow;
        }
        used += (usize)n;
    }
#endif
    return cex_demo_Error.ok;
}
