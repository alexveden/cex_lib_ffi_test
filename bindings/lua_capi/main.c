#include <lua.h>
#include <lauxlib.h>

#include <stdint.h>

#include "cex_demo.h"

typedef struct
{
    cex_demo_request_t req;
} request_ud_t;

static const char* request_meta = "cex_demo.Request";

static int
request_new(lua_State* L)
{
    lua_Integer value = luaL_optinteger(L, 1, 0);
    if (value < INT32_MIN || value > INT32_MAX) {
        return luaL_error(L, "value does not fit in int32");
    }
    request_ud_t* ud = (request_ud_t*)lua_newuserdata(L, sizeof *ud);
    ud->req.value = (int32_t)value;
    luaL_getmetatable(L, request_meta);
    lua_setmetatable(L, -2);
    return 1;
}

static int
request_get_value(lua_State* L)
{
    request_ud_t* ud = (request_ud_t*)luaL_checkudata(L, 1, request_meta);
    lua_pushinteger(L, ud->req.value);
    return 1;
}

static int
request_set_value(lua_State* L)
{
    request_ud_t* ud = (request_ud_t*)luaL_checkudata(L, 1, request_meta);
    lua_Integer value = luaL_checkinteger(L, 2);
    if (value < INT32_MIN || value > INT32_MAX) {
        return luaL_error(L, "value does not fit in int32");
    }
    ud->req.value = (int32_t)value;
    return 0;
}

static int
request_tostring(lua_State* L)
{
    request_ud_t* ud = (request_ud_t*)luaL_checkudata(L, 1, request_meta);
    lua_pushfstring(L, "Request(value=%d)", (int)ud->req.value);
    return 1;
}

static int
raise_cex(lua_State* L, cex_demo_exc_t exc)
{
    const char* name = "CexDemoError";
    if (exc == cex_demo_Error.argument) {
        name = "CexDemoArgumentError";
    } else if (exc == cex_demo_Error.overflow) {
        name = "CexDemoOverflowError";
    }
    lua_newtable(L);
    lua_pushstring(L, name);
    lua_setfield(L, -2, "name");
    lua_pushstring(L, exc);
    lua_setfield(L, -2, "message");
    return lua_error(L);
}

static int
lua_run(lua_State* L)
{
    cex_demo_request_t* req = NULL;
    if (!lua_isnoneornil(L, 1)) {
        req = &((request_ud_t*)luaL_checkudata(L, 1, request_meta))->req;
    }
    cex_demo_exc_t exc = cex_demo_run(req);
    if (exc != cex_demo_Error.ok) { return raise_cex(L, exc); }
    lua_pushnil(L);
    return 1;
}

static int
lua_traceback(lua_State* L)
{
    char buf[512];
    cex_demo_exc_t exc = cex_demo_traceback(buf, sizeof buf);
    if (exc != cex_demo_Error.ok) { return raise_cex(L, exc); }
    lua_pushstring(L, buf);
    return 1;
}

static const luaL_Reg module_functions[] = {
    { "Request", request_new },
    { "run", lua_run },
    { "traceback", lua_traceback },
    { NULL, NULL },
};

int
luaopen_cex_demo(lua_State* L)
{
    luaL_newmetatable(L, request_meta);
    lua_pushcfunction(L, request_get_value);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, request_set_value);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, request_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_register(L, NULL, module_functions);
    return 1;
}
