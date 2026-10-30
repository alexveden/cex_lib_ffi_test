#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>

#include "cex_demo.h"

namespace demo {

std::string
traceback()
{
    std::array<char, 512> buf{};
    cex_demo_exc_t exc = cex_demo_traceback(buf.data(), buf.size());
    if (exc == cex_demo_Error.argument) { throw std::invalid_argument(exc); }
    if (exc != cex_demo_Error.ok) { throw std::runtime_error(exc); }
    return std::string(buf.data());
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

    exc = cex_demo_run(nullptr);
    if (exc != cex_demo_Error.argument) {
        std::fprintf(stderr, "NULL request returned unexpected error\n");
        return 1;
    }
    std::printf("error=%s\n", exc);

    std::string traceback = demo::traceback();
    if (traceback.find("cex_demo__validate_request") == std::string::npos ||
        traceback.find("cex_demo_run") == std::string::npos) {
        std::fprintf(stderr, "traceback missing frames: %s", traceback.c_str());
        return 1;
    }
    std::printf("traceback=%s", traceback.c_str());
    return 0;
}
