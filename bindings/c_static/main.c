#include <stdint.h>
#include <stdio.h>

#include "cex_demo.h"

static cex_demo_exc_t
run_value(int32_t value, int32_t* out)
{
    cex_demo_request_t req = { .value = value };
    cex_demo_exc_t exc = cex_demo_run(&req);
    *out = req.value;
    return exc;
}

int
main(void)
{
    int32_t value = 0;
    cex_demo_exc_t exc = run_value(41, &value);
    if (exc != Error.ok || value != 42) {
        fprintf(stderr, "run failed: exc=%s value=%d\n", exc ? exc : "NULL", value);
        return 1;
    }
    printf("value=%d\n", value);

    exc = cex_demo_run(NULL);
    if (exc != Error.argument) {
        fprintf(stderr, "NULL request returned unexpected error\n");
        return 1;
    }
    printf("error=%s\n", exc);
    return 0;
}
