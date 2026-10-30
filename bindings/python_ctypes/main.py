import ctypes
import os
import sys


class Request(ctypes.Structure):
    _fields_ = [("value", ctypes.c_int32)]


class Errors(ctypes.Structure):
    _fields_ = [
        ("ok", ctypes.c_void_p),
        ("argument", ctypes.c_void_p),
        ("overflow", ctypes.c_void_p),
    ]


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
    _lib.cex_demo_traceback.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
    _lib.cex_demo_traceback.restype = ctypes.c_void_p
    _errors = Errors.in_dll(_lib, "cex_demo_Error")
    return _lib


def error_text(exc):
    return ctypes.string_at(exc).decode()


def main():
    lib = load()

    req = Request(41)
    exc = lib.cex_demo_run(ctypes.byref(req))
    if exc != _errors.ok or req.value != 42:
        print(f"run failed: exc={error_text(exc) if exc else 'NULL'} value={req.value}", file=sys.stderr)
        return 1
    print(f"value={req.value}")

    exc = lib.cex_demo_run(None)
    if exc != _errors.argument:
        print("NULL request returned unexpected error", file=sys.stderr)
        return 1
    print(f"error={error_text(exc)}")

    tb = ctypes.create_string_buffer(512)
    if lib.cex_demo_traceback(tb, len(tb)) != _errors.ok:
        print("traceback failed", file=sys.stderr)
        return 1
    text = tb.value.decode()
    if "cex_demo__validate_request" not in text or "cex_demo_run" not in text:
        print("traceback missing frames", file=sys.stderr)
        return 1
    print(f"traceback={text}", end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
