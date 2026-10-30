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
    tassert(cex_demo_Error.overflow == Error.overflow);
    return EOK;
}

test$case(cex_demo_traceback_null)
{
    char buf[8];
    tassert(cex_demo_traceback(NULL, 0) == cex_demo_Error.argument);
    tassert(cex_demo_traceback(buf, 0) == cex_demo_Error.argument);
    return EOK;
}

test$case(cex_demo_traceback_overflow)
{
    tassert(cex_demo_run(NULL) == cex_demo_Error.argument);
    char tiny[8];
    tassert(cex_demo_traceback(tiny, sizeof tiny) == cex_demo_Error.overflow);
    tassert(tiny[0] == '\0');

    char buf[512];
    tassert(cex_demo_traceback(buf, sizeof buf) == cex_demo_Error.ok);
    tassert(strstr(buf, "cex_demo_run") != NULL);
    return EOK;
}

test$case(cex_demo_traceback_contents)
{
    tassert(cex_demo_run(NULL) == cex_demo_Error.argument);
    char buf[512];
    tassert(cex_demo_traceback(buf, sizeof buf) == cex_demo_Error.ok);
    tassert(strstr(buf, "cex_demo__validate_request") != NULL);
    tassert(strstr(buf, "cex_demo_run") != NULL);
    tassert(strstr(buf, "ArgumentError") != NULL);
    return EOK;
}

test$case(cex_demo_traceback_empty)
{
    cex_demo_request_t req = { .value = 1 };
    tassert(cex_demo_run(&req) == cex_demo_Error.ok);
    char buf[8];
    tassert(cex_demo_traceback(buf, sizeof buf) == cex_demo_Error.ok);
    tassert(buf[0] == '\0');
    return EOK;
}

test$main();
