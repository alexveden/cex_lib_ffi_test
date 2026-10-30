#define CEX_IMPLEMENTATION
#include "cex.h"
#include "cex_demo.h"

int
main(void)
{
    cex_demo_request_t req = { .value = 41 };
    cex_demo_exc_t exc = cex_demo_run(&req);
    if (exc != cex_demo_Error.ok || req.value != 42) {
        io.printf("run failed: exc=%s value=%d\n", exc ? exc : "NULL", req.value);
        return 1;
    }
    io.printf("value=%d\n", req.value);

    exc = cex_demo_run(NULL);
    if (exc != cex_demo_Error.argument) {
        io.printf("NULL request returned unexpected error\n");
        return 1;
    }
    io.printf("error=%s\n", exc);

    char traceback[512];
    if (cex_demo_traceback(traceback, sizeof traceback) != cex_demo_Error.ok) {
        io.printf("traceback failed\n");
        return 1;
    }
    if (!str.find(traceback, "cex_demo__validate_request") || !str.find(traceback, "cex_demo_run")) {
        io.printf("traceback missing frames: %s", traceback);
        return 1;
    }
    io.printf("traceback=%s", traceback);
    return 0;
}
