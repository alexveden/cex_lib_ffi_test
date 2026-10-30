#define CEX_IMPLEMENTATION
#include "cex.h"

#define CEX_DEMO_BUILD
#include "cex_demo.h"

CEX_DEMO_API const cex_demo_errors_t cex_demo_Error = {
    .ok = NULL,
    .argument = _cex_errors_Error_argument,
    .overflow = _cex_errors_Error_overflow,
};

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
    return e$traceback_format(buf, buf_size);
}
