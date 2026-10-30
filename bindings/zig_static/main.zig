const std = @import("std");

const c = @cImport({
    @cInclude("cex_demo.h");
});

fn print(comptime fmt: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const text = std.fmt.bufPrint(&buf, fmt, args) catch return;
    std.Io.File.stdout().writeStreamingAll(std.Io.Threaded.global_single_threaded.io(), text) catch return;
}

pub fn main() !void {
    var req: c.cex_demo_request_t = .{ .value = 41 };
    const exc = c.cex_demo_run(&req);
    if (exc != c.cex_demo_Error.ok or req.value != 42) {
        print("run failed: exc={s} value={d}\n", .{ if (exc == null) "NULL" else std.mem.span(exc), req.value });
        return error.RunFailed;
    }
    print("value={d}\n", .{req.value});

    const null_exc = c.cex_demo_run(null);
    if (null_exc != c.cex_demo_Error.argument) {
        print("NULL request returned unexpected error\n", .{});
        return error.UnexpectedError;
    }
    print("error={s}\n", .{std.mem.span(null_exc)});

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
}
