#define CEX_IMPLEMENTATION
#define CEX_TEST
#include "cex.h"
#include "lib/cex_demo.c"

test$case(cex_demo_run_increments)
{
    cex_demo_request_t req = { .value = 41 };
    tassert(cex_demo_run(&req) == NULL);
    tassert_eq(req.value, 42);
    return EOK;
}

test$case(cex_demo_run_null)
{
    tassert(cex_demo_run(NULL) == Error.argument);
    return EOK;
}

test$main();
