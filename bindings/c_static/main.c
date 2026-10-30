#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cex_demo.h"

int
main(void)
{
    cex_demo_request_t req = { .value = 41 };
    cex_demo_exc_t exc = cex_demo_run(&req);
    if (exc != Error.ok || req.value != 42) {
        fprintf(stderr, "run failed: exc=%s value=%d\n", exc ? exc : "NULL", req.value);
        return 1;
    }
    printf("value=%d\n", req.value);

    exc = cex_demo_run(NULL);
    if (exc != Error.argument) {
        fprintf(stderr, "NULL request returned unexpected error\n");
        return 1;
    }
    printf("error=%s\n", exc);

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
