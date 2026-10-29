#define CEX_IMPLEMENTATION
#define CEX_TEST
#include "cex.h"
#define CEX_DEMO_PREBUILT
#include "lib/cex_demo.c"

test$case(cex_demo_run_increments)
{
    cex_demo_request_t req = { .value = 41 };
    tassert(cex_demo_run(&req) == cex_demo_Error.ok);
    tassert_eq(req.value, 42);
    return EOK;
}

test$case(cex_demo_run_null)
{
    tassert(cex_demo_run(NULL) == cex_demo_Error.argument);
    return EOK;
}

test$case(cex_demo_error_consts)
{
    tassert(cex_demo_Error.ok == Error.ok);
    tassert(cex_demo_Error.argument == Error.argument);
    return EOK;
}

test$main();
