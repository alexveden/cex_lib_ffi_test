local ffi = require("ffi")

ffi.cdef[[
typedef const char* cex_demo_exc_t;
typedef struct {
    cex_demo_exc_t ok;
    cex_demo_exc_t argument;
    cex_demo_exc_t overflow;
} cex_demo_errors_t;
typedef struct { int32_t value; } cex_demo_request_t;
cex_demo_exc_t cex_demo_run(cex_demo_request_t* req);
cex_demo_exc_t cex_demo_traceback(char* buf, size_t buf_size);
extern const cex_demo_errors_t cex_demo_Error;
]]

local library_names = {
    OSX = "build/lib/libcex_demo.dylib",
    Windows = "build/lib/libcex_demo.dll",
}

local lib = ffi.load(library_names[jit.os] or "build/lib/libcex_demo.so")
local errors = lib.cex_demo_Error

local function fail(message)
    io.stderr:write(message .. "\n")
    os.exit(1)
end

local req = ffi.new("cex_demo_request_t", 41)
local exc = lib.cex_demo_run(req)
if exc ~= errors.ok or req.value ~= 42 then
    local text = exc == ffi.NULL and "NULL" or ffi.string(exc)
    fail(("run failed: exc=%s value=%d"):format(text, req.value))
end
print(("value=%d"):format(req.value))

exc = lib.cex_demo_run(nil)
if exc ~= errors.argument then
    fail("NULL request returned unexpected error")
end
print("error=" .. ffi.string(exc))

local buf = ffi.new("char[512]")
if lib.cex_demo_traceback(buf, 512) ~= errors.ok then
    fail("traceback failed")
end
local tb = ffi.string(buf)
if not tb:find("cex_demo__validate_request", 1, true) or not tb:find("cex_demo_run", 1, true) then
    fail("traceback missing frames")
end
io.write("traceback=" .. tb)
