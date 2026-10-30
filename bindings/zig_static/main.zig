const std = @import("std");

const c = @cImport({
    @cInclude("cex_demo.h");
});

fn print(comptime fmt: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const text = std.fmt.bufPrint(&buf, fmt, args) catch return;
    std.Io.File.stdout().writeStreamingAll(std.Io.Threaded.global_single_threaded.io(), text) catch return;
}

fn check_error(exc: c.cex_demo_exc_t) !void {
    if (exc == c.cex_demo_Error.argument) return error.Argument;
    if (exc != c.cex_demo_Error.ok) return error.Unknown;
}

fn run(value: ?i32) !i32 {
    var req: c.cex_demo_request_t = undefined;
    var req_ptr: ?*c.cex_demo_request_t = null;
    if (value) |v| {
        req = .{ .value = v };
        req_ptr = &req;
    }
    try check_error(c.cex_demo_run(req_ptr));
    return if (req_ptr != null) req.value else 0;
}

pub fn main() !void {
    var req: c.cex_demo_request_t = .{ .value = 41 };
    const exc = c.cex_demo_run(&req);
    if (exc != c.cex_demo_Error.ok or req.value != 42) {
        print("run failed: exc={s} value={d}\n", .{ if (exc == null) "NULL" else std.mem.span(exc), req.value });
        return error.RunFailed;
    }
    print("value={d}\n", .{req.value});

    if (try run(41) != 42) {
        print("wrapper failed\n", .{});
        return error.WrapperFailed;
    }

    const null_exc = c.cex_demo_run(null);
    if (null_exc != c.cex_demo_Error.argument) {
        print("NULL request returned unexpected error\n", .{});
        return error.UnexpectedError;
    }
    print("error={s}\n", .{std.mem.span(null_exc)});

    if (c.cex_demo_traceback(null, 0) != c.cex_demo_Error.argument) {
        print("NULL traceback buffer returned unexpected error\n", .{});
        return error.UnexpectedError;
    }
    var tiny: [8]u8 = undefined;
    if (c.cex_demo_traceback(&tiny, tiny.len) != c.cex_demo_Error.overflow) {
        print("small traceback buffer returned unexpected error\n", .{});
        return error.UnexpectedError;
    }
    var tb: [512]u8 = undefined;
    if (c.cex_demo_traceback(&tb, tb.len) != c.cex_demo_Error.ok) {
        print("traceback failed\n", .{});
        return error.UnexpectedError;
    }
    const text = std.mem.sliceTo(&tb, 0);
    if (std.mem.indexOf(u8, text, "cex_demo__validate_request") == null or
        std.mem.indexOf(u8, text, "cex_demo_run") == null)
    {
        print("traceback missing frames\n", .{});
        return error.UnexpectedError;
    }
    print("traceback={s}", .{text});

    if (run(null)) |_| {
        print("wrapper NULL request unexpectedly succeeded\n", .{});
        return error.UnexpectedSuccess;
    } else |_| {}
}
