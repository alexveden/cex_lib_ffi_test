#define CEX_IMPLEMENTATION
#include "cex.h"

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
    };
}
#else
CEX_DEMO_API const cex_demo_errors_t cex_demo_Error = {
    .ok = Error.ok,
    .argument = Error.argument,
};
#endif

cex_demo_exc_t
cex_demo_run(cex_demo_request_t* req)
{
    if (!req) { return cex_demo_Error.argument; }
    req->value++;
    return NULL;
}
