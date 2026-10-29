#define CEX_IMPLEMENTATION
#include "cex.h"

#define CEX_DEMO_BUILD
#include "cex_demo.h"

#if defined(_WIN32)
__declspec(dllimport) HANDLE __stdcall CreateThread(
    void* lpThreadAttributes,
    size_t dwStackSize,
    DWORD(__stdcall* lpStartAddress)(void*),
    void* lpParameter,
    DWORD dwCreationFlags,
    DWORD* lpThreadId
);
#else
#    include <pthread.h>
#endif

_Static_assert(sizeof(cex_demo_point_t) == 8, "point layout");
_Static_assert(sizeof(cex_demo_padded_t) == 40, "padded layout");
_Static_assert(sizeof(cex_demo_mode_t) == 4, "enum is int-sized");

struct cex_demo_counter
{
    int32_t value;
};

#if defined(CEX_DEMO_PREBUILT)
CEX_DEMO_API cex_demo_errors_t cex_demo_Error;

__attribute__((constructor)) static void
cex_demo__init_errors(void)
{
    cex_demo_Error = (cex_demo_errors_t){
        .ok = Error.ok,
        .argument = Error.argument,
        .overflow = Error.overflow,
    };
}
#else
CEX_DEMO_API const cex_demo_errors_t cex_demo_Error = {
    .ok = Error.ok,
    .argument = Error.argument,
    .overflow = Error.overflow,
};
#endif

cex_demo_version_t
cex_demo_version(void)
{
    return (cex_demo_version_t){
        .major = 1, .minor = 0, .patch = 0, .struct_size = (uint32_t)sizeof(cex_demo_version_t)
    };
}

size_t
cex_demo_sizeof_padded(void)
{
    return sizeof(cex_demo_padded_t);
}

int8_t
cex_demo_id_i8(int8_t v)
{
    return v;
}

uint64_t
cex_demo_id_u64(uint64_t v)
{
    return v;
}

double
cex_demo_mix(double a, float b, int8_t c, uint8_t d)
{
    return a + (double)b + (double)c + (double)d;
}

bool
cex_demo_is_odd(uint64_t v)
{
    return (v & 1u) != 0;
}

size_t
cex_demo_str_len(const char* s)
{
    return s ? strlen(s) : 0;
}

size_t
cex_demo_str_concat_need(const char* a, const char* b)
{
    if (!a) { a = ""; }
    if (!b) { b = ""; }
    size_t la = strlen(a);
    size_t lb = strlen(b);
    if (la > SIZE_MAX - lb - 1) { return 0; }
    return la + lb + 1;
}

int
cex_demo_str_concat(char* out, size_t out_len, const char* a, const char* b)
{
    if (!out || out_len == 0) { return -1; }
    size_t need = cex_demo_str_concat_need(a, b);
    if (need == 0 || need > out_len) {
        out[0] = '\0';
        return -1;
    }
    if (!a) { a = ""; }
    if (!b) { b = ""; }
    size_t la = strlen(a);
    memcpy(out, a, la);
    memcpy(out + la, b, strlen(b) + 1);
    return 0;
}

int64_t
cex_demo_sum_i32(const int32_t* arr, size_t len)
{
    if (!arr) { return 0; }
    int64_t sum = 0;
    for (size_t i = 0; i < len; i++) { sum += arr[i]; }
    return sum;
}

void
cex_demo_scale_i32(int32_t* arr, size_t len, int32_t factor)
{
    if (!arr) { return; }
    for (size_t i = 0; i < len; i++) { arr[i] *= factor; }
}

cex_demo_point_t
cex_demo_point_add(cex_demo_point_t a, cex_demo_point_t b)
{
    return (cex_demo_point_t){ .x = a.x + b.x, .y = a.y + b.y };
}

void
cex_demo_point_scale(cex_demo_point_t* p, int32_t factor)
{
    if (!p) { return; }
    p->x *= factor;
    p->y *= factor;
}

cex_demo_padded_t
cex_demo_padded_make(uint8_t tag, int64_t big, double d, const char* name)
{
    cex_demo_padded_t out;
    memset(&out, 0, sizeof(out));
    out.tag = tag;
    out.big = big;
    out.d = d;
    if (name) {
        size_t n = strlen(name);
        if (n > sizeof(out.name) - 1) { n = sizeof(out.name) - 1; }
        memcpy(out.name, name, n);
    }
    return out;
}

cex_demo_mode_t
cex_demo_mode_next(cex_demo_mode_t mode)
{
    switch (mode) {
        case CEX_DEMO_MODE_A: return CEX_DEMO_MODE_B;
        case CEX_DEMO_MODE_B: return CEX_DEMO_MODE_C;
        default: return CEX_DEMO_MODE_A;
    }
}

int
cex_demo_divmod(int32_t a, int32_t b, int32_t* out_q, int32_t* out_r)
{
    if (b == 0 || !out_q || !out_r) { return -1; }
    *out_q = a / b;
    *out_r = a % b;
    return 0;
}

cex_demo_counter_t*
cex_demo_counter_new(int32_t start)
{
    cex_demo_counter_t* c = mem$new(mem$, cex_demo_counter_t);
    if (!c) { return NULL; }
    c->value = start;
    return c;
}

int32_t
cex_demo_counter_next(cex_demo_counter_t* c)
{
    if (!c) { return 0; }
    return c->value++;
}

void
cex_demo_counter_free(cex_demo_counter_t* c)
{
    if (!c) { return; }
    mem$free(mem$, c);
}

char*
cex_demo_str_dup(const char* s)
{
    if (!s) { return NULL; }
    return str.clone((char*)s, mem$);
}

void
cex_demo_free(void* p)
{
    if (!p) { return; }
    mem$free(mem$, p);
}

char*
cex_demo_str_dup_with(const char* s, cex_demo_malloc_fn m, cex_demo_free_fn f)
{
    (void)f;
    if (!s || !m) { return NULL; }
    size_t n = strlen(s) + 1;
    char* p = (char*)m(n);
    if (!p) { return NULL; }
    memcpy(p, s, n);
    return p;
}

char*
cex_demo_hazard_lib_alloc(const char* s)
{
    return cex_demo_str_dup(s);
}

int
cex_demo_hazard_take(char* p, cex_demo_free_fn f)
{
    if (!p || !f) { return -1; }
    f(p);
    return 0;
}

int32_t
cex_demo_apply(int32_t* arr, size_t len, cex_demo_cb_fn cb, void* user_data)
{
    if (!cb) { return -1; }
    if (!arr || len == 0) { return 0; }
    int64_t sum = 0;
    for (size_t i = 0; i < len; i++) { sum += cb(arr[i], user_data); }
    return (int32_t)sum;
}

typedef struct
{
    cex_demo_cb_fn cb;
    void* user_data;
    int32_t value;
    int32_t result;
} cex_demo__thread_ctx;

#if defined(_WIN32)
static DWORD __stdcall
cex_demo__thread_main(void* arg)
{
    cex_demo__thread_ctx* ctx = (cex_demo__thread_ctx*)arg;
    ctx->result = ctx->cb(ctx->value, ctx->user_data);
    return 0;
}
#else
static void*
cex_demo__thread_main(void* arg)
{
    cex_demo__thread_ctx* ctx = (cex_demo__thread_ctx*)arg;
    ctx->result = ctx->cb(ctx->value, ctx->user_data);
    return NULL;
}
#endif

int32_t
cex_demo_call_on_thread(cex_demo_cb_fn cb, void* user_data, int32_t value)
{
    if (!cb) { return -1; }
    cex_demo__thread_ctx ctx = { .cb = cb, .user_data = user_data, .value = value, .result = -1 };
#if defined(_WIN32)
    HANDLE t = CreateThread(NULL, 0, cex_demo__thread_main, &ctx, 0, NULL);
    if (!t) { return -1; }
    WaitForSingleObject(t, 0xFFFFFFFFu);
    CloseHandle(t);
#else
    pthread_t t;
    if (pthread_create(&t, NULL, cex_demo__thread_main, &ctx) != 0) { return -1; }
    pthread_join(t, NULL);
#endif
    return ctx.result;
}

const char*
cex_demo_parse_i32_exc(const char* s, int32_t* out)
{
    if (!s || !out) { return cex_demo_Error.argument; }
    i32 v = 0;
    Exc err = str.convert.to_i32((char*)s, &v);
    if (err) { return err; }
    *out = (int32_t)v;
    return NULL;
}

int32_t
cex_demo_parse_i32_status(const char* s, int32_t* out)
{
    return cex_demo_error_code(cex_demo_parse_i32_exc(s, out));
}

int32_t
cex_demo_parse_i32_hybrid(const char* s, int32_t* out, const char** out_exc)
{
    const char* exc = cex_demo_parse_i32_exc(s, out);
    if (out_exc) { *out_exc = exc; }
    return cex_demo_error_code(exc);
}

const char*
cex_demo_error_string(const char* exc)
{
    return exc ? exc : "OK";
}

int32_t
cex_demo_error_code(const char* exc)
{
    if (!exc) { return CEX_DEMO_ERR_OK; }
    if (exc == cex_demo_Error.argument) { return CEX_DEMO_ERR_ARGUMENT; }
    if (exc == cex_demo_Error.overflow) { return CEX_DEMO_ERR_OVERFLOW; }
    return CEX_DEMO_ERR_UNKNOWN;
}

const char*
cex_demo_strerror(int32_t code)
{
    switch (code) {
        case CEX_DEMO_ERR_OK: return "OK";
        case CEX_DEMO_ERR_ARGUMENT: return "ArgumentError";
        case CEX_DEMO_ERR_OVERFLOW: return "OverflowError";
        case CEX_DEMO_ERR_NOT_FOUND: return "NotFoundError";
        case CEX_DEMO_ERR_MEMORY: return "MemoryError";
        case CEX_DEMO_ERR_OS: return "OSError";
        default: return "UnknownError";
    }
}

#if CEX_TRACEBACK_VERBOSITY >= 1 && CEX_TRACEBACK_VERBOSITY <= 2
static _Thread_local char cex_demo__traceback_buf[4096];
#endif

const char*
cex_demo_last_traceback(void)
{
#if CEX_TRACEBACK_VERBOSITY >= 1 && CEX_TRACEBACK_VERBOSITY <= 2
    usize off = 0;
    cex_demo__traceback_buf[0] = '\0';
    for (u32 i = 0; i < _cex_errors_traceback_data_array.len; i++) {
        const _cex_errors_traceback_s* f = &_cex_errors_traceback_data_array.items[i];
#    if CEX_TRACEBACK_VERBOSITY == 2
        int w = snprintf(
            cex_demo__traceback_buf + off,
            sizeof(cex_demo__traceback_buf) - off,
            "#%u (%s:%u %s()) [%s] %s\n",
            i,
            f->file,
            f->line,
            f->func,
            f->err ? f->err : "OK",
            f->msg ? f->msg : ""
        );
#    else
        int w = snprintf(
            cex_demo__traceback_buf + off,
            sizeof(cex_demo__traceback_buf) - off,
            "#%u (%s:%u) [%s]\n",
            i,
            f->file,
            f->line,
            f->err ? f->err : "OK"
        );
#    endif
        if (w < 0) { break; }
        off += (usize)w;
        if (off >= sizeof(cex_demo__traceback_buf)) { break; }
    }
    return cex_demo__traceback_buf;
#else
    return "";
#endif
}
