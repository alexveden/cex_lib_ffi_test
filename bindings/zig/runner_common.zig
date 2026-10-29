const std = @import("std");

pub const Version = extern struct {
    major: u32,
    minor: u32,
    patch: u32,
    struct_size: u32,
};

pub const Point = extern struct {
    x: i32,
    y: i32,
};

pub const Padded = extern struct {
    tag: u8,
    big: i64,
    d: f64,
    name: [16]u8,
};

pub const MallocFn = *const fn (usize) callconv(.c) ?*anyopaque;
pub const FreeFn = *const fn (?*anyopaque) callconv(.c) void;
pub const CbFn = *const fn (i32, ?*anyopaque) callconv(.c) i32;

pub const Api = struct {
    version: *const fn () callconv(.c) Version,
    sizeof_padded: *const fn () callconv(.c) usize,
    id_i8: *const fn (i8) callconv(.c) i8,
    id_u64: *const fn (u64) callconv(.c) u64,
    mix: *const fn (f64, f32, i8, u8) callconv(.c) f64,
    is_odd: *const fn (u64) callconv(.c) bool,
    str_len: *const fn ([*c]const u8) callconv(.c) usize,
    str_concat: *const fn ([*c]u8, usize, [*c]const u8, [*c]const u8) callconv(.c) c_int,
    str_concat_need: *const fn ([*c]const u8, [*c]const u8) callconv(.c) usize,
    sum_i32: *const fn ([*c]const i32, usize) callconv(.c) i64,
    scale_i32: *const fn ([*c]i32, usize, i32) callconv(.c) void,
    point_add: *const fn (Point, Point) callconv(.c) Point,
    point_scale: *const fn (*Point, i32) callconv(.c) void,
    padded_make: *const fn (u8, i64, f64, [*c]const u8) callconv(.c) Padded,
    mode_next: *const fn (c_int) callconv(.c) c_int,
    divmod: *const fn (i32, i32, *i32, *i32) callconv(.c) c_int,
    counter_new: *const fn (i32) callconv(.c) ?*anyopaque,
    counter_next: *const fn (?*anyopaque) callconv(.c) i32,
    counter_free: *const fn (?*anyopaque) callconv(.c) void,
    str_dup: *const fn ([*c]const u8) callconv(.c) ?[*:0]u8,
    free: *const fn (?*anyopaque) callconv(.c) void,
    str_dup_with: *const fn ([*c]const u8, MallocFn, FreeFn) callconv(.c) ?[*:0]u8,
    hazard_lib_alloc: *const fn ([*c]const u8) callconv(.c) ?[*:0]u8,
    hazard_take: *const fn (?*anyopaque, FreeFn) callconv(.c) c_int,
    apply: *const fn ([*c]i32, usize, CbFn, ?*anyopaque) callconv(.c) i32,
    call_on_thread: *const fn (CbFn, ?*anyopaque, i32) callconv(.c) i32,
    parse_i32_exc: *const fn ([*c]const u8, *i32) callconv(.c) ?[*:0]const u8,
    parse_i32_status: *const fn ([*c]const u8, *i32) callconv(.c) i32,
    parse_i32_hybrid: *const fn ([*c]const u8, *i32, *?[*:0]const u8) callconv(.c) i32,
    error_string: *const fn (?[*:0]const u8) callconv(.c) [*:0]const u8,
    error_code: *const fn (?[*:0]const u8) callconv(.c) i32,
    strerror: *const fn (i32) callconv(.c) [*:0]const u8,
    last_traceback: *const fn () callconv(.c) [*:0]const u8,
    Err_ok: *const ?[*:0]const u8,
    Err_argument: *const ?[*:0]const u8,
    Err_overflow: *const ?[*:0]const u8,
    Err_not_found: *const ?[*:0]const u8,
    Err_memory: *const ?[*:0]const u8,
};

fn cstr(buf: []const u8) []const u8 {
    if (std.mem.indexOfScalar(u8, buf, 0)) |i| return buf[0..i];
    return buf;
}

fn zstr(src: []const u8, buf: []u8) ![:0]const u8 {
    if (src.len >= buf.len) return error.TooLong;
    @memcpy(buf[0..src.len], src);
    buf[src.len] = 0;
    return buf[0..src.len :0];
}

fn parseCsv(s: []const u8, out: []i32) !usize {
    if (s.len == 0) return 0;
    var it = std.mem.splitScalar(u8, s, ',');
    var n: usize = 0;
    while (it.next()) |v| {
        if (n >= out.len) return error.TooMany;
        out[n] = try std.fmt.parseInt(i32, v, 10);
        n += 1;
    }
    return n;
}

fn parsePoint(s: []const u8) !Point {
    var it = std.mem.splitScalar(u8, s, ',');
    const xs = it.next() orelse return error.BadPoint;
    const ys = it.next() orelse return error.BadPoint;
    return .{
        .x = try std.fmt.parseInt(i32, xs, 10),
        .y = try std.fmt.parseInt(i32, ys, 10),
    };
}

fn selectErrConst(api: *const Api, name: []const u8) ?[*:0]const u8 {
    if (std.mem.eql(u8, name, "ok")) return api.Err_ok.*;
    if (std.mem.eql(u8, name, "argument")) return api.Err_argument.*;
    if (std.mem.eql(u8, name, "overflow")) return api.Err_overflow.*;
    if (std.mem.eql(u8, name, "not_found")) return api.Err_not_found.*;
    if (std.mem.eql(u8, name, "memory")) return api.Err_memory.*;
    return null;
}

fn cbAdd(value: i32, user: ?*anyopaque) callconv(.c) i32 {
    const p: *i32 = @ptrCast(@alignCast(user.?));
    return value + p.*;
}

fn mallocFn(size: usize) callconv(.c) ?*anyopaque {
    return std.c.malloc(size);
}

fn freeFn(ptr: ?*anyopaque) callconv(.c) void {
    std.c.free(ptr);
}

fn formatCsv(vals: []const i32, out: []u8) ![]const u8 {
    var off: usize = 0;
    for (vals, 0..) |v, i| {
        const sep = if (i == 0) "" else ",";
        const w = try std.fmt.bufPrint(out[off..], "{s}{d}", .{ sep, v });
        off += w.len;
    }
    return out[0..off];
}

fn runCase(api: *const Api, kind: []const u8, args: []const u8, out: []u8) ![]const u8 {
    var arg_it = std.mem.splitScalar(u8, args, '|');
    var a: [8][]const u8 = undefined;
    var na: usize = 0;
    while (arg_it.next()) |v| {
        if (na < a.len) {
            a[na] = v;
            na += 1;
        }
    }

    if (std.mem.eql(u8, kind, "version")) {
        const v = api.version();
        return std.fmt.bufPrint(out, "{d},{d},{d},{d}", .{ v.major, v.minor, v.patch, v.struct_size });
    }
    if (std.mem.eql(u8, kind, "sizeof_padded")) {
        return std.fmt.bufPrint(out, "{d}", .{api.sizeof_padded()});
    }
    if (std.mem.eql(u8, kind, "id_i8")) {
        return std.fmt.bufPrint(out, "{d}", .{api.id_i8(try std.fmt.parseInt(i8, a[0], 10))});
    }
    if (std.mem.eql(u8, kind, "id_u64")) {
        return std.fmt.bufPrint(out, "{d}", .{api.id_u64(try std.fmt.parseInt(u64, a[0], 10))});
    }
    if (std.mem.eql(u8, kind, "mix")) {
        const v = api.mix(
            try std.fmt.parseFloat(f64, a[0]),
            try std.fmt.parseFloat(f32, a[1]),
            try std.fmt.parseInt(i8, a[2], 10),
            try std.fmt.parseInt(u8, a[3], 10),
        );
        return std.fmt.bufPrint(out, "{d:.3}", .{v});
    }
    if (std.mem.eql(u8, kind, "is_odd")) {
        const v = try std.fmt.parseInt(u64, a[0], 10);
        return if (api.is_odd(v)) "1" else "0";
    }
    if (std.mem.eql(u8, kind, "str_len")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        return std.fmt.bufPrint(out, "{d}", .{api.str_len(z.ptr)});
    }
    if (std.mem.eql(u8, kind, "str_len_null")) {
        return std.fmt.bufPrint(out, "{d}", .{api.str_len(null)});
    }
    if (std.mem.eql(u8, kind, "str_concat") or std.mem.eql(u8, kind, "str_concat_small")) {
        var za_buf: [256]u8 = undefined;
        var zb_buf: [256]u8 = undefined;
        const za = try zstr(a[0], &za_buf);
        const zb = try zstr(a[1], &zb_buf);
        const out_len = if (std.mem.eql(u8, kind, "str_concat_small"))
            try std.fmt.parseInt(usize, a[2], 10)
        else
            256;
        var buf: [256]u8 = undefined;
        const rc = api.str_concat(@ptrCast(&buf), out_len, za.ptr, zb.ptr);
        return std.fmt.bufPrint(out, "{d}:{s}", .{ rc, cstr(&buf) });
    }
    if (std.mem.eql(u8, kind, "str_concat_need")) {
        var za_buf: [256]u8 = undefined;
        var zb_buf: [256]u8 = undefined;
        const za = try zstr(a[0], &za_buf);
        const zb = try zstr(a[1], &zb_buf);
        return std.fmt.bufPrint(out, "{d}", .{api.str_concat_need(za.ptr, zb.ptr)});
    }
    if (std.mem.eql(u8, kind, "sum_i32")) {
        var vals: [64]i32 = undefined;
        const n = try parseCsv(a[0], &vals);
        return std.fmt.bufPrint(out, "{d}", .{api.sum_i32(&vals, n)});
    }
    if (std.mem.eql(u8, kind, "scale_i32")) {
        var vals: [64]i32 = undefined;
        const n = try parseCsv(a[0], &vals);
        api.scale_i32(&vals, n, try std.fmt.parseInt(i32, a[1], 10));
        return formatCsv(vals[0..n], out);
    }
    if (std.mem.eql(u8, kind, "point_add")) {
        const p = api.point_add(try parsePoint(a[0]), try parsePoint(a[1]));
        return std.fmt.bufPrint(out, "{d},{d}", .{ p.x, p.y });
    }
    if (std.mem.eql(u8, kind, "point_scale")) {
        var p = try parsePoint(a[0]);
        api.point_scale(&p, try std.fmt.parseInt(i32, a[1], 10));
        return std.fmt.bufPrint(out, "{d},{d}", .{ p.x, p.y });
    }
    if (std.mem.eql(u8, kind, "padded_make")) {
        var name_buf: [256]u8 = undefined;
        const name = try zstr(a[3], &name_buf);
        const p = api.padded_make(
            try std.fmt.parseInt(u8, a[0], 10),
            try std.fmt.parseInt(i64, a[1], 10),
            try std.fmt.parseFloat(f64, a[2]),
            name.ptr,
        );
        return std.fmt.bufPrint(out, "{d},{d},{d:.3},{s}", .{ p.tag, p.big, p.d, cstr(&p.name) });
    }
    if (std.mem.eql(u8, kind, "mode_next")) {
        return std.fmt.bufPrint(out, "{d}", .{api.mode_next(try std.fmt.parseInt(c_int, a[0], 10))});
    }
    if (std.mem.eql(u8, kind, "divmod")) {
        var q: i32 = 0;
        var r: i32 = 0;
        const rc = api.divmod(
            try std.fmt.parseInt(i32, a[0], 10),
            try std.fmt.parseInt(i32, a[1], 10),
            &q,
            &r,
        );
        return std.fmt.bufPrint(out, "{d}:{d}:{d}", .{ rc, q, r });
    }
    if (std.mem.eql(u8, kind, "counter")) {
        const c = api.counter_new(try std.fmt.parseInt(i32, a[0], 10));
        const count = try std.fmt.parseInt(usize, a[1], 10);
        var off: usize = 0;
        for (0..count) |i| {
            const sep = if (i == 0) "" else ",";
            const w = try std.fmt.bufPrint(out[off..], "{s}{d}", .{ sep, api.counter_next(c) });
            off += w.len;
        }
        api.counter_free(c);
        return out[0..off];
    }
    if (std.mem.eql(u8, kind, "str_dup") or std.mem.eql(u8, kind, "hazard_lib_alloc")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        const p = if (std.mem.eql(u8, kind, "str_dup"))
            api.str_dup(z.ptr)
        else
            api.hazard_lib_alloc(z.ptr);
        const s = if (p) |q| std.mem.span(q) else "NULL";
        const res = try std.fmt.bufPrint(out, "{s}", .{s});
        api.free(if (p) |q| @ptrCast(q) else null);
        return res;
    }
    if (std.mem.eql(u8, kind, "str_dup_with")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        const p = api.str_dup_with(z.ptr, &mallocFn, &freeFn);
        const s = if (p) |q| std.mem.span(q) else "NULL";
        const res = try std.fmt.bufPrint(out, "{s}", .{s});
        if (p) |q| std.c.free(@ptrCast(q));
        return res;
    }
    if (std.mem.eql(u8, kind, "hazard_take")) {
        const raw = a[0];
        const len = raw.len + 1;
        const mem = std.c.malloc(len) orelse return error.OutOfMemory;
        const bytes: [*]u8 = @ptrCast(mem);
        @memcpy(bytes[0..raw.len], raw);
        bytes[raw.len] = 0;
        const rc = api.hazard_take(mem, &freeFn);
        return std.fmt.bufPrint(out, "{d}", .{rc});
    }
    if (std.mem.eql(u8, kind, "apply")) {
        var vals: [64]i32 = undefined;
        const n = try parseCsv(a[0], &vals);
        var add = try std.fmt.parseInt(i32, a[1], 10);
        const res = api.apply(&vals, n, &cbAdd, @ptrCast(&add));
        return std.fmt.bufPrint(out, "{d}", .{res});
    }
    if (std.mem.eql(u8, kind, "call_on_thread")) {
        var add = try std.fmt.parseInt(i32, a[1], 10);
        const res = api.call_on_thread(
            &cbAdd,
            @ptrCast(&add),
            try std.fmt.parseInt(i32, a[0], 10),
        );
        return std.fmt.bufPrint(out, "{d}", .{res});
    }
    if (std.mem.eql(u8, kind, "parse_exc")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        var v: i32 = 0;
        const exc = api.parse_i32_exc(z.ptr, &v);
        const code = api.error_code(exc);
        const s = std.mem.span(api.error_string(exc));
        return std.fmt.bufPrint(out, "{d}:{s}", .{ code, s });
    }
    if (std.mem.eql(u8, kind, "parse_status")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        var v: i32 = 0;
        return std.fmt.bufPrint(out, "{d}", .{api.parse_i32_status(z.ptr, &v)});
    }
    if (std.mem.eql(u8, kind, "parse_hybrid")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        var v: i32 = 0;
        var exc: ?[*:0]const u8 = null;
        const code = api.parse_i32_hybrid(z.ptr, &v, &exc);
        const s = std.mem.span(api.error_string(exc));
        return std.fmt.bufPrint(out, "{d}:{s}", .{ code, s });
    }
    if (std.mem.eql(u8, kind, "err_const")) {
        const p = selectErrConst(api, a[0]);
        const s = if (p) |q| std.mem.span(q) else "NULL";
        return std.fmt.bufPrint(out, "{s}:{s}", .{ a[0], s });
    }
    if (std.mem.eql(u8, kind, "error_code")) {
        return std.fmt.bufPrint(out, "{d}", .{api.error_code(selectErrConst(api, a[0]))});
    }
    if (std.mem.eql(u8, kind, "strerror")) {
        const code = try std.fmt.parseInt(i32, a[0], 10);
        return std.fmt.bufPrint(out, "{d}:{s}", .{ code, std.mem.span(api.strerror(code)) });
    }
    if (std.mem.eql(u8, kind, "traceback_contains")) {
        var zbuf: [256]u8 = undefined;
        const z = try zstr(a[0], &zbuf);
        var v: i32 = 0;
        _ = api.parse_i32_exc(z.ptr, &v);
        const tb = std.mem.span(api.last_traceback());
        return if (std.mem.indexOf(u8, tb, "ArgumentError") != null) "1" else "0";
    }
    return std.fmt.bufPrint(out, "FAIL:unknown kind {s}", .{kind});
}

pub fn dispatch(api: *const Api, cases_path: [:0]const u8) !void {
    var buf: [1 << 16]u8 = undefined;
    const f = std.c.fopen(cases_path.ptr, "r") orelse return error.OpenFailed;
    const n = std.c.fread(buf[0..].ptr, 1, buf.len - 1, f);
    _ = std.c.fclose(f);
    const data = buf[0..n];

    var lines = std.mem.splitScalar(u8, data, '\n');
    while (lines.next()) |raw| {
        const line = std.mem.trimEnd(u8, raw, "\r");
        if (line.len == 0 or line[0] == '#') continue;

        var fields: [4][]const u8 = undefined;
        var fi: usize = 0;
        var start: usize = 0;
        var i: usize = 0;
        while (i <= line.len and fi < 4) : (i += 1) {
            if (i == line.len or line[i] == '\t') {
                fields[fi] = line[start..i];
                fi += 1;
                start = i + 1;
            }
        }
        if (fi < 4) continue;

        var res_buf: [1024]u8 = undefined;
        const result = runCase(api, fields[1], fields[2], &res_buf) catch |err| blk: {
            break :blk std.fmt.bufPrint(&res_buf, "FAIL:{s}", .{@errorName(err)}) catch "FAIL";
        };
        var out_buf: [1200]u8 = undefined;
        const z = try std.fmt.bufPrintZ(&out_buf, "{s}\t{s}", .{ fields[0], result });
        _ = std.c.printf("%s\n", z.ptr);
    }
}
