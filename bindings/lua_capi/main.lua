local suffix = package.config:sub(1, 1) == "\\" and "dll" or "so"
package.cpath = ("build/bindings/lua_capi/?.%s;"):format(suffix) .. package.cpath

local cex_demo = require("cex_demo")

local function fail(message)
    io.stderr:write(message .. "\n")
    os.exit(1)
end

local req = cex_demo.Request(41)
cex_demo.run(req)
if req.value ~= 42 then
    fail(("run failed: value=%d"):format(req.value))
end
print(("value=%d"):format(req.value))

local ok, err = pcall(cex_demo.run, nil)
if ok or type(err) ~= "table" or err.name ~= "CexDemoArgumentError" then
    fail("NULL request returned unexpected error")
end
print("error=" .. err.message)

local tb = cex_demo.traceback()
if not tb:find("cex_demo__validate_request", 1, true) or not tb:find("cex_demo_run", 1, true) then
    fail("traceback missing frames")
end
io.write("traceback=" .. tb)
