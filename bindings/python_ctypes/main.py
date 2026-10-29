import ctypes
import os
import sys


class Request(ctypes.Structure):
    _fields_ = [("value", ctypes.c_int32)]


class Errors(ctypes.Structure):
    _fields_ = [("ok", ctypes.c_void_p), ("argument", ctypes.c_void_p)]


def library_path():
    if sys.platform == "darwin":
        return "build/lib/libcex_demo.dylib"
    if os.name == "nt":
        return "build/lib/libcex_demo.dll"
    return "build/lib/libcex_demo.so"


_lib = None
_errors = None


def load():
    global _lib, _errors
    _lib = ctypes.CDLL(library_path())
    _lib.cex_demo_run.argtypes = [ctypes.POINTER(Request)]
    _lib.cex_demo_run.restype = ctypes.c_void_p
    _errors = Errors.in_dll(_lib, "cex_demo_Error")
    return _lib


def error_text(exc):
    return ctypes.string_at(exc).decode()


def run(value):
    req = None if value is None else Request(value)
    exc = _lib.cex_demo_run(ctypes.byref(req) if req else None)
    if exc == _errors.argument:
        raise RuntimeError(error_text(exc))
    if exc != _errors.ok:
        raise RuntimeError(error_text(exc))
    return req.value if req else 0


def main():
    lib = load()

    req = Request(41)
    exc = lib.cex_demo_run(ctypes.byref(req))
    if exc != _errors.ok or req.value != 42:
        print(f"run failed: exc={error_text(exc) if exc else 'NULL'} value={req.value}", file=sys.stderr)
        return 1
    print(f"value={req.value}")

    if run(41) != 42:
        print("wrapper failed", file=sys.stderr)
        return 1

    exc = lib.cex_demo_run(None)
    if exc != _errors.argument:
        print("NULL request returned unexpected error", file=sys.stderr)
        return 1
    print(f"error={error_text(exc)}")

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
