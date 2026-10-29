const fs = require("fs");
const koffi = require("koffi");

koffi.struct("cex_demo_version_t", {
  major: "uint32_t",
  minor: "uint32_t",
  patch: "uint32_t",
  struct_size: "uint32_t",
});
koffi.struct("cex_demo_point_t", { x: "int32_t", y: "int32_t" });
koffi.struct("cex_demo_padded_t", {
  tag: "uint8_t",
  big: "int64_t",
  d: "double",
  name: "char[16]",
});
koffi.enumeration("cex_demo_mode_t", {
  CEX_DEMO_MODE_A: 0,
  CEX_DEMO_MODE_B: 1,
  CEX_DEMO_MODE_C: 2,
});
koffi.opaque("cex_demo_counter_t");
koffi.proto("int32_t cex_demo_cb_fn(int32_t value, void *user_data)");
koffi.proto("void *cex_demo_malloc_fn(size_t size)");
koffi.proto("void cex_demo_free_fn(void *ptr)");

function loadLib(path) {
  const lib = koffi.load(path);
  const libc = koffi.load(null);
  const fn = {
    version: lib.func("cex_demo_version_t cex_demo_version(void)"),
    sizeof_padded: lib.func("size_t cex_demo_sizeof_padded(void)"),
    id_i8: lib.func("int8_t cex_demo_id_i8(int8_t v)"),
    id_u64: lib.func("uint64_t cex_demo_id_u64(uint64_t v)"),
    mix: lib.func("double cex_demo_mix(double a, float b, int8_t c, uint8_t d)"),
    is_odd: lib.func("bool cex_demo_is_odd(uint64_t v)"),
    str_len: lib.func("size_t cex_demo_str_len(const char *s)"),
    str_concat: lib.func(
      "int cex_demo_str_concat(void *out, size_t out_len, const char *a, const char *b)",
    ),
    str_concat_need: lib.func(
      "size_t cex_demo_str_concat_need(const char *a, const char *b)",
    ),
    sum_i32: lib.func("int64_t cex_demo_sum_i32(const int32_t *arr, size_t len)"),
    scale_i32: lib.func(
      "void cex_demo_scale_i32(int32_t *arr, size_t len, int32_t factor)",
    ),
    point_add: lib.func(
      "cex_demo_point_t cex_demo_point_add(cex_demo_point_t a, cex_demo_point_t b)",
    ),
    point_scale: lib.func(
      "void cex_demo_point_scale(cex_demo_point_t *p, int32_t factor)",
    ),
    padded_make: lib.func(
      "cex_demo_padded_t cex_demo_padded_make(uint8_t tag, int64_t big, double d, const char *name)",
    ),
    mode_next: lib.func("cex_demo_mode_t cex_demo_mode_next(cex_demo_mode_t mode)"),
    divmod: lib.func(
      "int cex_demo_divmod(int32_t a, int32_t b, int32_t *q, int32_t *r)",
    ),
    counter_new: lib.func("void *cex_demo_counter_new(int32_t start)"),
    counter_next: lib.func("int32_t cex_demo_counter_next(void *c)"),
    counter_free: lib.func("void cex_demo_counter_free(void *c)"),
    str_dup: lib.func("void *cex_demo_str_dup(const char *s)"),
    free: lib.func("void cex_demo_free(void *p)"),
    str_dup_with: lib.func(
      "void *cex_demo_str_dup_with(const char *s, cex_demo_malloc_fn *m, cex_demo_free_fn *f)",
    ),
    hazard_lib_alloc: lib.func("void *cex_demo_hazard_lib_alloc(const char *s)"),
    hazard_take: lib.func("int cex_demo_hazard_take(void *p, cex_demo_free_fn *f)"),
    apply: lib.func(
      "int32_t cex_demo_apply(int32_t *arr, size_t len, cex_demo_cb_fn *cb, void *user_data)",
    ),
    call_on_thread: lib.func(
      "int32_t cex_demo_call_on_thread(cex_demo_cb_fn *cb, void *user_data, int32_t value)",
    ),
    parse_exc: lib.func(
      "void *cex_demo_parse_i32_exc(const char *s, int32_t *out)",
    ),
    parse_status: lib.func(
      "int32_t cex_demo_parse_i32_status(const char *s, int32_t *out)",
    ),
    parse_hybrid: lib.func(
      "int32_t cex_demo_parse_i32_hybrid(const char *s, int32_t *out, void **out_exc)",
    ),
    error_string: lib.func("void *cex_demo_error_string(void *exc)"),
    error_code: lib.func("int32_t cex_demo_error_code(void *exc)"),
    strerror: lib.func("const char *cex_demo_strerror(int32_t code)"),
    last_traceback: lib.func("void *cex_demo_last_traceback(void)"),
    malloc: libc.func("void *malloc(size_t size)"),
    libc_free: libc.func("void free(void *ptr)"),
  };
  return { lib, fn };
}

function errConst(lib, name) {
  const sym = lib.symbol("cex_demo_Err_" + name);
  return koffi.decode(sym, "void *");
}

function strOrNull(ptr) {
  return ptr === null || ptr === 0n ? "NULL" : koffi.decode.string(ptr);
}

function call(ctx, kind, args) {
  const { lib, fn } = ctx;
  const a = args ? args.split("|") : [];
  switch (kind) {
    case "version": {
      const v = fn.version();
      return `${v.major},${v.minor},${v.patch},${v.struct_size}`;
    }
    case "sizeof_padded":
      return String(fn.sizeof_padded());
    case "id_i8":
      return String(fn.id_i8(Number(a[0])));
    case "id_u64":
      return String(fn.id_u64(BigInt(a[0])));
    case "mix":
      return fn
        .mix(Number(a[0]), Number(a[1]), Number(a[2]), Number(a[3]))
        .toFixed(3);
    case "is_odd":
      return fn.is_odd(BigInt(a[0])) ? "1" : "0";
    case "str_len":
      return String(fn.str_len(a[0]));
    case "str_len_null":
      return String(fn.str_len(null));
    case "str_concat":
    case "str_concat_small": {
      const outLen = kind === "str_concat_small" ? Number(a[2]) : 256;
      const buf = koffi.alloc("char", 256);
      const rc = fn.str_concat(buf, outLen, a[0], a[1]);
      return `${rc}:${koffi.decode.string(buf)}`;
    }
    case "str_concat_need":
      return String(fn.str_concat_need(a[0], a[1]));
    case "sum_i32": {
      const vals = a[0] ? a[0].split(",").map(Number) : [];
      const ptr = vals.length ? koffi.as(vals, "int32_t *") : null;
      return String(fn.sum_i32(ptr, vals.length));
    }
    case "scale_i32": {
      const vals = a[0] ? a[0].split(",").map(Number) : [];
      const ptr = koffi.alloc("int32_t", Math.max(vals.length, 1));
      if (vals.length) koffi.encode(ptr, "int32_t", vals, vals.length);
      fn.scale_i32(ptr, vals.length, Number(a[1]));
      return Array.from(koffi.decode(ptr, "int32_t", vals.length)).join(",");
    }
    case "point_add": {
      const [x1, y1] = a[0].split(",").map(Number);
      const [x2, y2] = a[1].split(",").map(Number);
      const p = fn.point_add({ x: x1, y: y1 }, { x: x2, y: y2 });
      return `${p.x},${p.y}`;
    }
    case "point_scale": {
      const [x, y] = a[0].split(",").map(Number);
      const p = koffi.alloc("cex_demo_point_t", 1);
      koffi.encode(p, "cex_demo_point_t", { x, y });
      fn.point_scale(p, Number(a[1]));
      const r = koffi.decode(p, "cex_demo_point_t");
      return `${r.x},${r.y}`;
    }
    case "padded_make": {
      const p = fn.padded_make(
        Number(a[0]),
        BigInt(a[1]),
        Number(a[2]),
        a[3],
      );
      return `${p.tag},${p.big},${Number(p.d).toFixed(3)},${p.name}`;
    }
    case "mode_next":
      return String(fn.mode_next(Number(a[0])));
    case "divmod": {
      const q = koffi.alloc("int32_t", 1);
      const r = koffi.alloc("int32_t", 1);
      const rc = fn.divmod(Number(a[0]), Number(a[1]), q, r);
      return `${rc}:${koffi.decode(q, "int32_t")}:${koffi.decode(r, "int32_t")}`;
    }
    case "counter": {
      const c = fn.counter_new(Number(a[0]));
      const vals = [];
      for (let i = 0; i < Number(a[1]); i++) vals.push(fn.counter_next(c));
      fn.counter_free(c);
      return vals.join(",");
    }
    case "str_dup":
    case "hazard_lib_alloc": {
      const f = kind === "str_dup" ? fn.str_dup : fn.hazard_lib_alloc;
      const p = f(a[0]);
      const s = strOrNull(p);
      fn.free(p);
      return s;
    }
    case "str_dup_with": {
      const m = (n) => fn.malloc(BigInt(n));
      const f = (p) => fn.libc_free(p);
      const p = fn.str_dup_with(a[0], m, f);
      const s = strOrNull(p);
      fn.libc_free(p);
      return s;
    }
    case "hazard_take": {
      const raw = Buffer.from(a[0] + "\0", "utf8");
      const p = fn.malloc(BigInt(raw.length));
      koffi.encode(p, "char", Array.from(raw), raw.length);
      return String(fn.hazard_take(p, (q) => fn.libc_free(q)));
    }
    case "apply": {
      const vals = a[0] ? a[0].split(",").map(Number) : [];
      const arr = koffi.alloc("int32_t", Math.max(vals.length, 1));
      if (vals.length) koffi.encode(arr, "int32_t", vals, vals.length);
      const user = koffi.alloc("int32_t", 1);
      koffi.encode(user, "int32_t", Number(a[1]));
      return String(
        fn.apply(arr, vals.length, (value, u) => value + koffi.decode(u, "int32_t"), user),
      );
    }
    case "call_on_thread":
      return "SKIP:koffi queues foreign-thread callbacks to the blocked main thread";
    case "parse_exc": {
      const out = koffi.alloc("int32_t", 1);
      const exc = fn.parse_exc(a[0], out);
      const code = fn.error_code(exc);
      const s = strOrNull(fn.error_string(exc));
      return `${code}:${s}`;
    }
    case "parse_status": {
      const out = koffi.alloc("int32_t", 1);
      return String(fn.parse_status(a[0], out));
    }
    case "parse_hybrid": {
      const out = koffi.alloc("int32_t", 1);
      const excp = koffi.alloc("void *", 1);
      const code = fn.parse_hybrid(a[0], out, excp);
      const exc = koffi.decode(excp, "void *");
      return `${code}:${strOrNull(fn.error_string(exc))}`;
    }
    case "err_const": {
      const p = errConst(lib, a[0]);
      return `${a[0]}:${strOrNull(p)}`;
    }
    case "error_code":
      return String(fn.error_code(errConst(lib, a[0])));
    case "strerror": {
      const code = Number(a[0]);
      return `${code}:${fn.strerror(code)}`;
    }
    case "traceback_contains": {
      const out = koffi.alloc("int32_t", 1);
      fn.parse_exc(a[0], out);
      const tb = koffi.decode.string(fn.last_traceback());
      return tb.includes("ArgumentError") ? "1" : "0";
    }
    default:
      return `FAIL:unknown kind ${kind}`;
  }
}

function main(argv) {
  if (argv.length < 4) {
    console.error("usage: koffi_runner.js cases.tsv libpath");
    return 2;
  }
  const ctx = loadLib(argv[3]);
  const lines = fs.readFileSync(argv[2], "utf8").split(/\r?\n/);
  for (const line of lines) {
    if (!line || line.startsWith("#")) continue;
    const parts = line.split("\t");
    if (parts.length < 4) continue;
    const [id, kind, args] = parts;
    let result;
    try {
      result = call(ctx, kind, args);
    } catch (err) {
      result = `FAIL:${err.name}:${err.message}`;
    }
    console.log(`${id}\t${result}`);
  }
  return 0;
}

if (require.main === module) {
  process.exitCode = main(process.argv);
}
