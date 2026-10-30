import os
import sys

ROOT = os.getcwd()
sys.path.insert(0, os.path.join(ROOT, "build", "cffi"))

from _cex_demo_cffi import ffi, lib  # noqa: E402


def error_text(exc):
    return ffi.string(exc).decode()


def run(value):
    req = ffi.NULL if value is None else ffi.new("cex_demo_request_t *", {"value": value})
    exc = lib.cex_demo_run(req)
    if exc == lib.cex_demo_Error.argument:
        raise RuntimeError(error_text(exc))
    if exc != lib.cex_demo_Error.ok:
        raise RuntimeError(error_text(exc))
    return req.value if req != ffi.NULL else 0


def main():
    req = ffi.new("cex_demo_request_t *", {"value": 41})
    exc = lib.cex_demo_run(req)
    if exc != lib.cex_demo_Error.ok or req.value != 42:
        print(
            f"run failed: exc={error_text(exc) if exc != ffi.NULL else 'NULL'} value={req.value}",
            file=sys.stderr,
        )
        return 1
    print(f"value={req.value}")

    if run(41) != 42:
        print("wrapper failed", file=sys.stderr)
        return 1

    exc = lib.cex_demo_run(ffi.NULL)
    if exc != lib.cex_demo_Error.argument:
        print("NULL request returned unexpected error", file=sys.stderr)
        return 1
    print(f"error={error_text(exc)}")

    if lib.cex_demo_traceback(ffi.NULL, 0) != lib.cex_demo_Error.argument:
        print("NULL traceback buffer returned unexpected error", file=sys.stderr)
        return 1
    tiny = ffi.new("char[]", 8)
    if lib.cex_demo_traceback(tiny, len(tiny)) != lib.cex_demo_Error.overflow:
        print("small traceback buffer returned unexpected error", file=sys.stderr)
        return 1
    tb = ffi.new("char[]", 512)
    if lib.cex_demo_traceback(tb, len(tb)) != lib.cex_demo_Error.ok:
        print("traceback failed", file=sys.stderr)
        return 1
    text = ffi.string(tb).decode()
    if "cex_demo__validate_request" not in text or "cex_demo_run" not in text:
        print("traceback missing frames", file=sys.stderr)
        return 1
    print(f"traceback={text}", end="")

    try:
        run(None)
    except RuntimeError:
        pass
    else:
        print("wrapper NULL request unexpectedly succeeded", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
