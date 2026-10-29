const std = @import("std");
const common = @import("runner_common.zig");

pub fn main(init: std.process.Init.Minimal) !void {
    var it = init.args.iterate();
    _ = it.next();
    const cases = it.next() orelse return error.Usage;
    const lib_path = it.next() orelse return error.Usage;

    var lib = try std.DynLib.open(lib_path);
    defer lib.close();

    var api: common.Api = undefined;
    inline for (std.meta.fields(common.Api)) |field| {
        const sym = std.fmt.comptimePrint("cex_demo_{s}", .{field.name});
        @field(api, field.name) = lib.lookup(field.type, sym) orelse {
            std.debug.print("missing symbol: {s}\n", .{sym});
            return error.MissingSymbol;
        };
    }

    try common.dispatch(&api, cases);
}
