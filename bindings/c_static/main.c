#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

    if (cex_demo_traceback(NULL, 0) != Error.argument) {
        fprintf(stderr, "NULL traceback buffer returned unexpected error\n");
        return 1;
    }
    char tiny[8];
    if (cex_demo_traceback(tiny, sizeof tiny) != Error.overflow) {
        fprintf(stderr, "small traceback buffer returned unexpected error\n");
        return 1;
    }
    char traceback[512];
    if (cex_demo_traceback(traceback, sizeof traceback) != Error.ok) {
        fprintf(stderr, "traceback failed\n");
        return 1;
    }
    if (!strstr(traceback, "cex_demo__validate_request") || !strstr(traceback, "cex_demo_run")) {
        fprintf(stderr, "traceback missing frames: %s", traceback);
        return 1;
    }
    printf("traceback=%s", traceback);
    return 0;
}
