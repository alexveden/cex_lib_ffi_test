#define CEX_IMPLEMENTATION
#include "cex.h"

#define CEX_DEMO_BUILD
#include "cex_demo.h"

cex_demo_exc_t
cex_demo_run(cex_demo_request_t* req)
{
    if (!req) { return Error.argument; }
    req->value++;
    return NULL;
}
