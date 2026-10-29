import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "build", "cffi"))

from _cex_demo_cffi import ffi, lib  # noqa: E402


def _s(ptr):
    return "NULL" if ptr == ffi.NULL else ffi.string(ptr).decode()


class Backend:
    def load(self, _path):
        self.libc = ffi.dlopen(None)

    def err_const(self, name):
        return getattr(lib, "cex_demo_Err_" + name)

    def call(self, kind, args):
        a = args.split("|") if args else []
        if kind == "version":
            v = lib.cex_demo_version()
            return f"{v.major},{v.minor},{v.patch},{v.struct_size}"
        if kind == "sizeof_padded":
            return str(lib.cex_demo_sizeof_padded())
        if kind == "id_i8":
            return str(lib.cex_demo_id_i8(int(a[0])))
        if kind == "id_u64":
            return str(lib.cex_demo_id_u64(int(a[0])))
        if kind == "mix":
            return f"{lib.cex_demo_mix(float(a[0]), float(a[1]), int(a[2]), int(a[3])):.3f}"
        if kind == "is_odd":
            return "1" if lib.cex_demo_is_odd(int(a[0])) else "0"
        if kind == "str_len":
            return str(lib.cex_demo_str_len(a[0].encode()))
        if kind == "str_len_null":
            return str(lib.cex_demo_str_len(ffi.NULL))
        if kind in ("str_concat", "str_concat_small"):
            out_len = int(a[2]) if kind == "str_concat_small" else 256
            buf = ffi.new("char[]", 256)
            rc = lib.cex_demo_str_concat(buf, out_len, a[0].encode(), a[1].encode())
            return f"{rc}:{ffi.string(buf).decode()}"
        if kind == "str_concat_need":
            return str(lib.cex_demo_str_concat_need(a[0].encode(), a[1].encode()))
        if kind == "sum_i32":
            vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
            arr = ffi.new("int32_t[]", vals) if vals else ffi.NULL
            return str(lib.cex_demo_sum_i32(arr, len(vals)))
        if kind == "scale_i32":
            vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
            arr = ffi.new("int32_t[]", vals) if vals else ffi.NULL
            lib.cex_demo_scale_i32(arr, len(vals), int(a[1]))
            return ",".join(str(arr[i]) for i in range(len(vals)))
        if kind == "point_add":
            x1, y1 = (int(v) for v in a[0].split(","))
            x2, y2 = (int(v) for v in a[1].split(","))
            p = lib.cex_demo_point_add(
                ffi.new("cex_demo_point_t *", {"x": x1, "y": y1})[0],
                ffi.new("cex_demo_point_t *", {"x": x2, "y": y2})[0],
            )
            return f"{p.x},{p.y}"
        if kind == "point_scale":
            x, y = (int(v) for v in a[0].split(","))
            p = ffi.new("cex_demo_point_t *", {"x": x, "y": y})
            lib.cex_demo_point_scale(p, int(a[1]))
            return f"{p.x},{p.y}"
        if kind == "padded_make":
            p = lib.cex_demo_padded_make(
                int(a[0]), int(a[1]), float(a[2]), a[3].encode()
            )
            return f"{p.tag},{p.big},{p.d:.3f},{ffi.string(p.name).decode()}"
        if kind == "mode_next":
            return str(lib.cex_demo_mode_next(int(a[0])))
        if kind == "divmod":
            q = ffi.new("int32_t *")
            r = ffi.new("int32_t *")
            rc = lib.cex_demo_divmod(int(a[0]), int(a[1]), q, r)
            return f"{rc}:{q[0]}:{r[0]}"
        if kind == "counter":
            c = lib.cex_demo_counter_new(int(a[0]))
            vals = [lib.cex_demo_counter_next(c) for _ in range(int(a[1]))]
            lib.cex_demo_counter_free(c)
            return ",".join(str(v) for v in vals)
        if kind in ("str_dup", "hazard_lib_alloc"):
            fn = (
                lib.cex_demo_str_dup
                if kind == "str_dup"
                else lib.cex_demo_hazard_lib_alloc
            )
            p = fn(a[0].encode())
            s = _s(p)
            lib.cex_demo_free(p)
            return s
        if kind == "str_dup_with":
            p = lib.cex_demo_str_dup_with(a[0].encode(), self.libc.malloc, self.libc.free)
            s = _s(p)
            self.libc.free(p)
            return s
        if kind == "hazard_take":
            raw = a[0].encode() + b"\0"
            p = self.libc.malloc(len(raw))
            ffi.memmove(p, raw, len(raw))
            return str(lib.cex_demo_hazard_take(ffi.cast("char *", p), self.libc.free))
        if kind in ("apply", "call_on_thread"):
            add = int(a[1])
            user = ffi.new("int32_t *", add)

            @ffi.callback("int32_t(int32_t, void *)")
            def cb(value, _user):
                return value + ffi.cast("int32_t *", _user)[0]

            if kind == "apply":
                vals = [int(x) for x in a[0].split(",")] if a and a[0] else []
                arr = ffi.new("int32_t[]", vals) if vals else ffi.NULL
                return str(
                    lib.cex_demo_apply(arr, len(vals), cb, ffi.cast("void *", user))
                )
            return str(
                lib.cex_demo_call_on_thread(
                    cb, ffi.cast("void *", user), int(a[0])
                )
            )
        if kind == "parse_exc":
            v = ffi.new("int32_t *")
            exc = lib.cex_demo_parse_i32_exc(a[0].encode(), v)
            return f"{lib.cex_demo_error_code(exc)}:{_s(lib.cex_demo_error_string(exc))}"
        if kind == "parse_status":
            v = ffi.new("int32_t *")
            return str(lib.cex_demo_parse_i32_status(a[0].encode(), v))
        if kind == "parse_hybrid":
            v = ffi.new("int32_t *")
            exc = ffi.new("const char **")
            code = lib.cex_demo_parse_i32_hybrid(a[0].encode(), v, exc)
            return f"{code}:{_s(lib.cex_demo_error_string(exc[0]))}"
        if kind == "err_const":
            p = self.err_const(a[0])
            return f"{a[0]}:{_s(p)}"
        if kind == "error_code":
            return str(lib.cex_demo_error_code(self.err_const(a[0])))
        if kind == "strerror":
            code = int(a[0])
            return f"{code}:{ffi.string(lib.cex_demo_strerror(code)).decode()}"
        if kind == "traceback_contains":
            v = ffi.new("int32_t *")
            lib.cex_demo_parse_i32_exc(a[0].encode(), v)
            tb = ffi.string(lib.cex_demo_last_traceback())
            return "1" if b"ArgumentError" in tb else "0"
        return f"FAIL:unknown kind {kind}"


def main(argv):
    if len(argv) < 3:
        print("usage: cffi_runner.py cases.tsv [ignored]", file=sys.stderr)
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
