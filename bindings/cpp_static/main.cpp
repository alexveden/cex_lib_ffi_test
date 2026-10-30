#include <cstdint>
#include <cstdio>
#include <optional>
#include <stdexcept>

#include "cex_demo.h"

namespace demo {

int32_t
run(std::optional<int32_t> value)
{
    if (!value) { throw std::invalid_argument(cex_demo_Error.argument); }
    cex_demo_request_t req{ .value = *value };
    cex_demo_exc_t exc = cex_demo_run(&req);
    if (exc == cex_demo_Error.argument) { throw std::invalid_argument(exc); }
    if (exc != cex_demo_Error.ok) { throw std::runtime_error(exc); }
    return req.value;
}

} // namespace demo

int
main()
{
    cex_demo_request_t req{ .value = 41 };
    cex_demo_exc_t exc = cex_demo_run(&req);
    if (exc != cex_demo_Error.ok || req.value != 42) {
        std::fprintf(stderr, "run failed: exc=%s value=%d\n", exc ? exc : "NULL", req.value);
        return 1;
    }
    std::printf("value=%d\n", req.value);

    if (demo::run(41) != 42) {
        std::fprintf(stderr, "wrapper failed\n");
        return 1;
    }

    exc = cex_demo_run(nullptr);
    if (exc != cex_demo_Error.argument) {
        std::fprintf(stderr, "NULL request returned unexpected error\n");
        return 1;
    }
    std::printf("error=%s\n", exc);

    try {
        demo::run(std::nullopt);
    } catch (const std::invalid_argument&) {
        return 0;
    }
    std::fprintf(stderr, "wrapper NULL request unexpectedly succeeded\n");
    return 1;
}
