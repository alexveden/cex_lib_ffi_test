const std = @import("std");
const common = @import("runner_common.zig");

extern fn cex_demo_version() callconv(.c) common.Version;
extern fn cex_demo_sizeof_padded() callconv(.c) usize;
extern fn cex_demo_id_i8(v: i8) callconv(.c) i8;
extern fn cex_demo_id_u64(v: u64) callconv(.c) u64;
extern fn cex_demo_mix(a: f64, b: f32, c: i8, d: u8) callconv(.c) f64;
extern fn cex_demo_is_odd(v: u64) callconv(.c) bool;
extern fn cex_demo_str_len(s: [*c]const u8) callconv(.c) usize;
extern fn cex_demo_str_concat(out: [*c]u8, out_len: usize, a: [*c]const u8, b: [*c]const u8) callconv(.c) c_int;
extern fn cex_demo_str_concat_need(a: [*c]const u8, b: [*c]const u8) callconv(.c) usize;
extern fn cex_demo_sum_i32(arr: [*c]const i32, len: usize) callconv(.c) i64;
extern fn cex_demo_scale_i32(arr: [*c]i32, len: usize, factor: i32) callconv(.c) void;
extern fn cex_demo_point_add(a: common.Point, b: common.Point) callconv(.c) common.Point;
extern fn cex_demo_point_scale(p: *common.Point, factor: i32) callconv(.c) void;
extern fn cex_demo_padded_make(tag: u8, big: i64, d: f64, name: [*c]const u8) callconv(.c) common.Padded;
extern fn cex_demo_mode_next(mode: c_int) callconv(.c) c_int;
extern fn cex_demo_divmod(a: i32, b: i32, q: *i32, r: *i32) callconv(.c) c_int;
extern fn cex_demo_counter_new(start: i32) callconv(.c) ?*anyopaque;
extern fn cex_demo_counter_next(c: ?*anyopaque) callconv(.c) i32;
extern fn cex_demo_counter_free(c: ?*anyopaque) callconv(.c) void;
extern fn cex_demo_str_dup(s: [*c]const u8) callconv(.c) ?[*:0]u8;
extern fn cex_demo_free(p: ?*anyopaque) callconv(.c) void;
extern fn cex_demo_str_dup_with(s: [*c]const u8, m: common.MallocFn, f: common.FreeFn) callconv(.c) ?[*:0]u8;
extern fn cex_demo_hazard_lib_alloc(s: [*c]const u8) callconv(.c) ?[*:0]u8;
extern fn cex_demo_hazard_take(p: ?*anyopaque, f: common.FreeFn) callconv(.c) c_int;
extern fn cex_demo_apply(arr: [*c]i32, len: usize, cb: common.CbFn, user_data: ?*anyopaque) callconv(.c) i32;
extern fn cex_demo_call_on_thread(cb: common.CbFn, user_data: ?*anyopaque, value: i32) callconv(.c) i32;
extern fn cex_demo_parse_i32_exc(s: [*c]const u8, out: *i32) callconv(.c) ?[*:0]const u8;
extern fn cex_demo_parse_i32_status(s: [*c]const u8, out: *i32) callconv(.c) i32;
extern fn cex_demo_parse_i32_hybrid(s: [*c]const u8, out: *i32, out_exc: *?[*:0]const u8) callconv(.c) i32;
extern fn cex_demo_error_string(exc: ?[*:0]const u8) callconv(.c) [*:0]const u8;
extern fn cex_demo_error_code(exc: ?[*:0]const u8) callconv(.c) i32;
extern fn cex_demo_strerror(code: i32) callconv(.c) [*:0]const u8;
extern fn cex_demo_last_traceback() callconv(.c) [*:0]const u8;

extern var cex_demo_Err_ok: ?[*:0]const u8;
extern var cex_demo_Err_argument: ?[*:0]const u8;
extern var cex_demo_Err_overflow: ?[*:0]const u8;
extern var cex_demo_Err_not_found: ?[*:0]const u8;
extern var cex_demo_Err_memory: ?[*:0]const u8;

pub fn main(init: std.process.Init.Minimal) !void {
    var it = init.args.iterate();
    _ = it.next();
    const cases = it.next() orelse return error.Usage;

    const api = common.Api{
        .version = &cex_demo_version,
        .sizeof_padded = &cex_demo_sizeof_padded,
        .id_i8 = &cex_demo_id_i8,
        .id_u64 = &cex_demo_id_u64,
        .mix = &cex_demo_mix,
        .is_odd = &cex_demo_is_odd,
        .str_len = &cex_demo_str_len,
        .str_concat = &cex_demo_str_concat,
        .str_concat_need = &cex_demo_str_concat_need,
        .sum_i32 = &cex_demo_sum_i32,
        .scale_i32 = &cex_demo_scale_i32,
        .point_add = &cex_demo_point_add,
        .point_scale = &cex_demo_point_scale,
        .padded_make = &cex_demo_padded_make,
        .mode_next = &cex_demo_mode_next,
        .divmod = &cex_demo_divmod,
        .counter_new = &cex_demo_counter_new,
        .counter_next = &cex_demo_counter_next,
        .counter_free = &cex_demo_counter_free,
        .str_dup = &cex_demo_str_dup,
        .free = &cex_demo_free,
        .str_dup_with = &cex_demo_str_dup_with,
        .hazard_lib_alloc = &cex_demo_hazard_lib_alloc,
        .hazard_take = &cex_demo_hazard_take,
        .apply = &cex_demo_apply,
        .call_on_thread = &cex_demo_call_on_thread,
        .parse_i32_exc = &cex_demo_parse_i32_exc,
        .parse_i32_status = &cex_demo_parse_i32_status,
        .parse_i32_hybrid = &cex_demo_parse_i32_hybrid,
        .error_string = &cex_demo_error_string,
        .error_code = &cex_demo_error_code,
        .strerror = &cex_demo_strerror,
        .last_traceback = &cex_demo_last_traceback,
        .Err_ok = &cex_demo_Err_ok,
        .Err_argument = &cex_demo_Err_argument,
        .Err_overflow = &cex_demo_Err_overflow,
        .Err_not_found = &cex_demo_Err_not_found,
        .Err_memory = &cex_demo_Err_memory,
    };

    try common.dispatch(&api, cases);
}
