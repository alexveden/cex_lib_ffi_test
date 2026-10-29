import ctypes
import sys

c_int8 = ctypes.c_int8
c_uint8 = ctypes.c_uint8
c_int32 = ctypes.c_int32
c_uint32 = ctypes.c_uint32
c_int64 = ctypes.c_int64
c_uint64 = ctypes.c_uint64
c_size_t = ctypes.c_size_t
c_double = ctypes.c_double
c_float = ctypes.c_float
c_bool = ctypes.c_bool
c_char_p = ctypes.c_char_p
c_void_p = ctypes.c_void_p
POINTER = ctypes.POINTER


class Version(ctypes.Structure):
    _fields_ = [
        ("major", c_uint32),
        ("minor", c_uint32),
        ("patch", c_uint32),
        ("struct_size", c_uint32),
    ]


class Point(ctypes.Structure):
    _fields_ = [("x", c_int32), ("y", c_int32)]


class Padded(ctypes.Structure):
    _fields_ = [
        ("tag", c_uint8),
        ("big", c_int64),
        ("d", c_double),
        ("name", ctypes.c_char * 16),
    ]


CB = ctypes.CFUNCTYPE(c_int32, c_int32, c_void_p)


def _bytes(p):
    return ctypes.string_at(p).decode() if p else "NULL"


class Errors(ctypes.Structure):
    _fields_ = [
        ("ok", c_void_p),
        ("argument", c_void_p),
        ("overflow", c_void_p),
    ]


class Backend:
    def load(self, path):
        self.lib = ctypes.CDLL(path)
        self.libc = ctypes.CDLL(None)
        self.libc.malloc.restype = c_void_p
        self.libc.malloc.argtypes = [c_size_t]
        self.libc.free.restype = None
        self.libc.free.argtypes = [c_void_p]
        self._declare()
        self.errors = Errors.in_dll(self.lib, "cex_demo_Error")

    def _declare(self):
        L = self.lib
        L.cex_demo_version.restype = Version
        L.cex_demo_sizeof_padded.restype = c_size_t
        L.cex_demo_id_i8.argtypes = [c_int8]
        L.cex_demo_id_i8.restype = c_int8
        L.cex_demo_id_u64.argtypes = [c_uint64]
        L.cex_demo_id_u64.restype = c_uint64
        L.cex_demo_mix.argtypes = [c_double, c_float, c_int8, c_uint8]
        L.cex_demo_mix.restype = c_double
        L.cex_demo_is_odd.argtypes = [c_uint64]
        L.cex_demo_is_odd.restype = c_bool
        L.cex_demo_str_len.argtypes = [c_char_p]
        L.cex_demo_str_len.restype = c_size_t
        L.cex_demo_str_concat.argtypes = [c_void_p, c_size_t, c_char_p, c_char_p]
        L.cex_demo_str_concat.restype = ctypes.c_int
        L.cex_demo_str_concat_need.argtypes = [c_char_p, c_char_p]
        L.cex_demo_str_concat_need.restype = c_size_t
        L.cex_demo_sum_i32.argtypes = [POINTER(c_int32), c_size_t]
        L.cex_demo_sum_i32.restype = c_int64
        L.cex_demo_scale_i32.argtypes = [POINTER(c_int32), c_size_t, c_int32]
        L.cex_demo_scale_i32.restype = None
        L.cex_demo_point_add.argtypes = [Point, Point]
        L.cex_demo_point_add.restype = Point
        L.cex_demo_point_scale.argtypes = [POINTER(Point), c_int32]
        L.cex_demo_point_scale.restype = None
        L.cex_demo_padded_make.argtypes = [c_uint8, c_int64, c_double, c_char_p]
        L.cex_demo_padded_make.restype = Padded
        L.cex_demo_mode_next.argtypes = [ctypes.c_int]
        L.cex_demo_mode_next.restype = ctypes.c_int
        L.cex_demo_divmod.argtypes = [c_int32, c_int32, POINTER(c_int32), POINTER(c_int32)]
        L.cex_demo_divmod.restype = ctypes.c_int
        L.cex_demo_counter_new.argtypes = [c_int32]
        L.cex_demo_counter_new.restype = c_void_p
        L.cex_demo_counter_next.argtypes = [c_void_p]
        L.cex_demo_counter_next.restype = c_int32
        L.cex_demo_counter_free.argtypes = [c_void_p]
        L.cex_demo_counter_free.restype = None
        L.cex_demo_str_dup.argtypes = [c_char_p]
        L.cex_demo_str_dup.restype = c_void_p
        L.cex_demo_free.argtypes = [c_void_p]
        L.cex_demo_free.restype = None
        L.cex_demo_str_dup_with.argtypes = [c_char_p, c_void_p, c_void_p]
        L.cex_demo_str_dup_with.restype = c_void_p
        L.cex_demo_hazard_lib_alloc.argtypes = [c_char_p]
        L.cex_demo_hazard_lib_alloc.restype = c_void_p
        L.cex_demo_hazard_take.argtypes = [c_void_p, c_void_p]
        L.cex_demo_hazard_take.restype = ctypes.c_int
        L.cex_demo_apply.argtypes = [POINTER(c_int32), c_size_t, CB, c_void_p]
        L.cex_demo_apply.restype = c_int32
        L.cex_demo_call_on_thread.argtypes = [CB, c_void_p, c_int32]
        L.cex_demo_call_on_thread.restype = c_int32
        L.cex_demo_parse_i32_exc.argtypes = [c_char_p, POINTER(c_int32)]
        L.cex_demo_parse_i32_exc.restype = c_void_p
        L.cex_demo_parse_i32_status.argtypes = [c_char_p, POINTER(c_int32)]
        L.cex_demo_parse_i32_status.restype = c_int32
        L.cex_demo_parse_i32_hybrid.argtypes = [c_char_p, POINTER(c_int32), POINTER(c_void_p)]
        L.cex_demo_parse_i32_hybrid.restype = c_int32
        L.cex_demo_error_string.argtypes = [c_void_p]
        L.cex_demo_error_string.restype = c_void_p
        L.cex_demo_error_code.argtypes = [c_void_p]
        L.cex_demo_error_code.restype = c_int32
        L.cex_demo_strerror.argtypes = [c_int32]
        L.cex_demo_strerror.restype = c_char_p
        L.cex_demo_last_traceback.argtypes = []
        L.cex_demo_last_traceback.restype = c_char_p

    def err_const(self, name):
        return getattr(self.errors, name)

    def call(self, kind, args):
        a = args.split("|") if args else []
        L = self.lib
        if kind == "version":
            v = L.cex_demo_version()
            return f"{v.major},{v.minor},{v.patch},{v.struct_size}"
        if kind == "sizeof_padded":
            return str(L.cex_demo_sizeof_padded())
        if kind == "id_i8":
            return str(L.cex_demo_id_i8(int(a[0])))
        if kind == "id_u64":
            return str(L.cex_demo_id_u64(int(a[0])))
        if kind == "mix":
            return f"{L.cex_demo_mix(float(a[0]), float(a[1]), int(a[2]), int(a[3])):.3f}"
        if kind == "is_odd":
            return "1" if L.cex_demo_is_odd(int(a[0])) else "0"
        if kind == "str_len":
            return str(L.cex_demo_str_len(a[0].encode()))
        if kind == "str_len_null":
            return str(L.cex_demo_str_len(None))
        if kind in ("str_concat", "str_concat_small"):
            out_len = int(a[2]) if kind == "str_concat_small" else 256
            buf = ctypes.create_string_buffer(256)
            rc = L.cex_demo_str_concat(
                ctypes.cast(buf, c_void_p), out_len, a[0].encode(), a[1].encode()
            )
            return f"{rc}:{buf.value.decode()}"
        if kind == "str_concat_need":
            return str(L.cex_demo_str_concat_need(a[0].encode(), a[1].encode()))
        if kind == "sum_i32":
            vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
            arr = (c_int32 * len(vals))(*vals)
            return str(L.cex_demo_sum_i32(arr, len(vals)))
        if kind == "scale_i32":
            vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
            arr = (c_int32 * len(vals))(*vals)
            L.cex_demo_scale_i32(arr, len(vals), int(a[1]))
            return ",".join(str(x) for x in arr)
        if kind == "point_add":
            x1, y1 = (int(v) for v in a[0].split(","))
            x2, y2 = (int(v) for v in a[1].split(","))
            p = L.cex_demo_point_add(Point(x1, y1), Point(x2, y2))
            return f"{p.x},{p.y}"
        if kind == "point_scale":
            x, y = (int(v) for v in a[0].split(","))
            p = Point(x, y)
            L.cex_demo_point_scale(ctypes.byref(p), int(a[1]))
            return f"{p.x},{p.y}"
        if kind == "padded_make":
            p = L.cex_demo_padded_make(int(a[0]), int(a[1]), float(a[2]), a[3].encode())
            return f"{p.tag},{p.big},{p.d:.3f},{p.name.decode()}"
        if kind == "mode_next":
            return str(L.cex_demo_mode_next(int(a[0])))
        if kind == "divmod":
            q = c_int32(0)
            r = c_int32(0)
            rc = L.cex_demo_divmod(int(a[0]), int(a[1]), ctypes.byref(q), ctypes.byref(r))
            return f"{rc}:{q.value}:{r.value}"
        if kind == "counter":
            c = L.cex_demo_counter_new(int(a[0]))
            vals = [L.cex_demo_counter_next(c) for _ in range(int(a[1]))]
            L.cex_demo_counter_free(c)
            return ",".join(str(v) for v in vals)
        if kind in ("str_dup", "hazard_lib_alloc"):
            fn = L.cex_demo_str_dup if kind == "str_dup" else L.cex_demo_hazard_lib_alloc
            p = fn(a[0].encode())
            s = _bytes(p)
            L.cex_demo_free(p)
            return s
        if kind == "str_dup_with":
            p = L.cex_demo_str_dup_with(
                a[0].encode(),
                ctypes.cast(self.libc.malloc, c_void_p),
                ctypes.cast(self.libc.free, c_void_p),
            )
            s = _bytes(p)
            self.libc.free(p)
            return s
        if kind == "hazard_take":
            raw = a[0].encode() + b"\0"
            p = self.libc.malloc(len(raw))
            ctypes.memmove(p, raw, len(raw))
            rc = L.cex_demo_hazard_take(p, ctypes.cast(self.libc.free, c_void_p))
            return str(rc)
        if kind in ("apply", "call_on_thread"):
            add = int(a[1])
            add_ptr = ctypes.pointer(c_int32(add))

            @CB
            def cb(value, user):
                return value + ctypes.cast(user, POINTER(c_int32))[0]

            if kind == "apply":
                vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
                arr = (c_int32 * len(vals))(*vals)
                return str(L.cex_demo_apply(arr, len(vals), cb, ctypes.cast(add_ptr, c_void_p)))
            return str(
                L.cex_demo_call_on_thread(cb, ctypes.cast(add_ptr, c_void_p), int(a[0]))
            )
        if kind == "parse_exc":
            v = c_int32(0)
            exc = L.cex_demo_parse_i32_exc(a[0].encode(), ctypes.byref(v))
            return f"{L.cex_demo_error_code(exc)}:{_bytes(L.cex_demo_error_string(exc))}"
        if kind == "parse_status":
            v = c_int32(0)
            return str(L.cex_demo_parse_i32_status(a[0].encode(), ctypes.byref(v)))
        if kind == "parse_hybrid":
            v = c_int32(0)
            exc = c_void_p()
            code = L.cex_demo_parse_i32_hybrid(
                a[0].encode(), ctypes.byref(v), ctypes.byref(exc)
            )
            return f"{code}:{_bytes(L.cex_demo_error_string(exc))}"
        if kind == "err_const":
            p = self.err_const(a[0])
            return f"{a[0]}:{_bytes(p)}"
        if kind == "err_identity":
            v = c_int32(0)
            exc = L.cex_demo_parse_i32_exc(a[0].encode(), ctypes.byref(v))
            return "1" if exc == self.err_const(a[1]) else "0"
        if kind == "error_code":
            return str(L.cex_demo_error_code(self.err_const(a[0])))
        if kind == "strerror":
            code = int(a[0])
            return f"{code}:{L.cex_demo_strerror(code).decode()}"
        if kind == "traceback_contains":
            v = c_int32(0)
            L.cex_demo_parse_i32_exc(a[0].encode(), ctypes.byref(v))
            tb = L.cex_demo_last_traceback()
            return "1" if tb and b"ArgumentError" in tb else "0"
        return f"FAIL:unknown kind {kind}"


def main(argv):
    if len(argv) < 3:
        print("usage: ctypes_runner.py cases.tsv libpath", file=sys.stderr)
        return 2
    backend = Backend()
    backend.load(argv[2])
    with open(argv[1], encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 4:
                continue
            cid, kind, args = parts[0], parts[1], parts[2]
            try:
                result = backend.call(kind, args)
            except Exception as exc:  # noqa: BLE001
                result = f"FAIL:{type(exc).__name__}:{exc}"
            print(f"{cid}\t{result}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
