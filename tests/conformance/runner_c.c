#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CEX_DEMO_STATIC
#    define CEX_DEMO_STATIC
#endif
#include "cex_demo.h"

#ifdef CEX_DEMO_DYNAMIC
#    ifdef _WIN32
#        include <windows.h>
static HMODULE cex_demo_lib;
#        define OPEN_LIB(p) LoadLibraryA(p)
#        define SYM(h, n) ((void*)GetProcAddress((h), (n)))
#    else
#        include <dlfcn.h>
static void* cex_demo_lib;
#        define OPEN_LIB(p) dlopen((p), RTLD_NOW)
#        define SYM(h, n) dlsym((h), (n))
#    endif
#    define F(name) ((__typeof__(&name))SYM(cex_demo_lib, #name))
#else
#    define F(name) name
#endif

static char*
dup_str(const char* s)
{
    size_t n = strlen(s) + 1;
    char* p = malloc(n);
    if (p) { memcpy(p, s, n); }
    return p;
}

static int
split_pipe(char* s, char** out, int max)
{
    int n = 0;
    if (max <= 0) { return 0; }
    out[n++] = s;
    for (char* p = s; *p; p++) {
        if (*p != '|') { continue; }
        *p = '\0';
        if (n < max) { out[n++] = p + 1; }
    }
    return n;
}

static int
parse_csv_i32(const char* s, int32_t* out, int max)
{
    int n = 0;
    if (!s || !*s) { return 0; }
    char* copy = dup_str(s);
    char* p = copy;
    while (p && n < max) {
        char* comma = strchr(p, ',');
        if (comma) { *comma = '\0'; }
        out[n++] = (int32_t)strtol(p, NULL, 10);
        p = comma ? comma + 1 : NULL;
    }
    free(copy);
    return n;
}

static void
format_csv_i32(const int32_t* v, int n, char* out, size_t out_len)
{
    size_t off = 0;
    out[0] = '\0';
    for (int i = 0; i < n; i++) {
        int w = snprintf(out + off, out_len - off, i ? ",%d" : "%d", v[i]);
        if (w < 0) { return; }
        off += (size_t)w;
    }
}

static cex_demo_point_t
parse_point(const char* s)
{
    cex_demo_point_t p = { 0, 0 };
    char* copy = dup_str(s);
    char* comma = strchr(copy, ',');
    if (comma) {
        *comma = '\0';
        p.x = (int32_t)strtol(copy, NULL, 10);
        p.y = (int32_t)strtol(comma + 1, NULL, 10);
    }
    free(copy);
    return p;
}

static int32_t
cb_add(int32_t value, void* user_data)
{
    return value + *(int32_t*)user_data;
}

#ifdef CEX_DEMO_DYNAMIC
static const char*
err_const_by_name(const char* name)
{
    char sym_name[64];
    snprintf(sym_name, sizeof(sym_name), "cex_demo_Err_%s", name);
    void* sym = SYM(cex_demo_lib, sym_name);
    if (!sym) { return NULL; }
    return *(const char* const*)sym;
}
#else
static const char*
err_const_by_name(const char* name)
{
    if (!strcmp(name, "ok")) { return cex_demo_Err_ok; }
    if (!strcmp(name, "argument")) { return cex_demo_Err_argument; }
    if (!strcmp(name, "overflow")) { return cex_demo_Err_overflow; }
    if (!strcmp(name, "not_found")) { return cex_demo_Err_not_found; }
    if (!strcmp(name, "memory")) { return cex_demo_Err_memory; }
    return NULL;
}
#endif

static void
run_case(const char* kind, char* args, char* res, size_t res_len)
{
    char abuf[2048];
    snprintf(abuf, sizeof(abuf), "%s", args);
    char* a[8] = { 0 };
    int na = split_pipe(abuf, a, 8);
    (void)na;

    if (!strcmp(kind, "version")) {
        cex_demo_version_t v = F(cex_demo_version)();
        snprintf(res, res_len, "%u,%u,%u,%u", v.major, v.minor, v.patch, v.struct_size);
    } else if (!strcmp(kind, "sizeof_padded")) {
        snprintf(res, res_len, "%zu", F(cex_demo_sizeof_padded)());
    } else if (!strcmp(kind, "id_i8")) {
        snprintf(res, res_len, "%d", (int)F(cex_demo_id_i8)((int8_t)strtol(a[0], NULL, 10)));
    } else if (!strcmp(kind, "id_u64")) {
        snprintf(res, res_len, "%llu", (unsigned long long)F(cex_demo_id_u64)(strtoull(a[0], NULL, 10)));
    } else if (!strcmp(kind, "mix")) {
        double v = F(cex_demo_mix)(
            strtod(a[0], NULL), strtof(a[1], NULL), (int8_t)strtol(a[2], NULL, 10), (uint8_t)strtol(a[3], NULL, 10)
        );
        snprintf(res, res_len, "%.3f", v);
    } else if (!strcmp(kind, "is_odd")) {
        snprintf(res, res_len, "%d", F(cex_demo_is_odd)(strtoull(a[0], NULL, 10)) ? 1 : 0);
    } else if (!strcmp(kind, "str_len")) {
        snprintf(res, res_len, "%zu", F(cex_demo_str_len)(a[0]));
    } else if (!strcmp(kind, "str_len_null")) {
        snprintf(res, res_len, "%zu", F(cex_demo_str_len)(NULL));
    } else if (!strcmp(kind, "str_concat")) {
        char out[256] = { 0 };
        int rc = F(cex_demo_str_concat)(out, sizeof(out), a[0], a[1]);
        snprintf(res, res_len, "%d:%s", rc, out);
    } else if (!strcmp(kind, "str_concat_small")) {
        char out[256] = { 0 };
        int rc = F(cex_demo_str_concat)(out, (size_t)strtoull(a[2], NULL, 10), a[0], a[1]);
        snprintf(res, res_len, "%d:%s", rc, out);
    } else if (!strcmp(kind, "str_concat_need")) {
        snprintf(res, res_len, "%zu", F(cex_demo_str_concat_need)(a[0], a[1]));
    } else if (!strcmp(kind, "sum_i32")) {
        int32_t v[64];
        int n = parse_csv_i32(a[0], v, 64);
        snprintf(res, res_len, "%lld", (long long)F(cex_demo_sum_i32)(v, (size_t)n));
    } else if (!strcmp(kind, "scale_i32")) {
        int32_t v[64];
        int n = parse_csv_i32(a[0], v, 64);
        F(cex_demo_scale_i32)(v, (size_t)n, (int32_t)strtol(a[1], NULL, 10));
        format_csv_i32(v, n, res, res_len);
    } else if (!strcmp(kind, "point_add")) {
        cex_demo_point_t p = F(cex_demo_point_add)(parse_point(a[0]), parse_point(a[1]));
        snprintf(res, res_len, "%d,%d", p.x, p.y);
    } else if (!strcmp(kind, "point_scale")) {
        cex_demo_point_t p = parse_point(a[0]);
        F(cex_demo_point_scale)(&p, (int32_t)strtol(a[1], NULL, 10));
        snprintf(res, res_len, "%d,%d", p.x, p.y);
    } else if (!strcmp(kind, "padded_make")) {
        cex_demo_padded_t p = F(cex_demo_padded_make)(
            (uint8_t)strtol(a[0], NULL, 10), strtoll(a[1], NULL, 10), strtod(a[2], NULL), a[3]
        );
        snprintf(res, res_len, "%u,%lld,%.3f,%s", p.tag, (long long)p.big, p.d, p.name);
    } else if (!strcmp(kind, "mode_next")) {
        snprintf(res, res_len, "%d", (int)F(cex_demo_mode_next)((cex_demo_mode_t)strtol(a[0], NULL, 10)));
    } else if (!strcmp(kind, "divmod")) {
        int32_t q = 0;
        int32_t r = 0;
        int rc = F(cex_demo_divmod)((int32_t)strtol(a[0], NULL, 10), (int32_t)strtol(a[1], NULL, 10), &q, &r);
        snprintf(res, res_len, "%d:%d:%d", rc, q, r);
    } else if (!strcmp(kind, "counter")) {
        cex_demo_counter_t* c = F(cex_demo_counter_new)((int32_t)strtol(a[0], NULL, 10));
        long n = strtol(a[1], NULL, 10);
        size_t off = 0;
        res[0] = '\0';
        for (long i = 0; i < n; i++) {
            int w = snprintf(res + off, res_len - off, i ? ",%d" : "%d", F(cex_demo_counter_next)(c));
            if (w < 0) { break; }
            off += (size_t)w;
        }
        F(cex_demo_counter_free)(c);
    } else if (!strcmp(kind, "str_dup")) {
        char* p = F(cex_demo_str_dup)(a[0]);
        snprintf(res, res_len, "%s", p ? p : "NULL");
        F(cex_demo_free)(p);
    } else if (!strcmp(kind, "str_dup_with")) {
        char* p = F(cex_demo_str_dup_with)(a[0], malloc, free);
        snprintf(res, res_len, "%s", p ? p : "NULL");
        free(p);
    } else if (!strcmp(kind, "hazard_lib_alloc")) {
        char* p = F(cex_demo_hazard_lib_alloc)(a[0]);
        snprintf(res, res_len, "%s", p ? p : "NULL");
        F(cex_demo_free)(p);
    } else if (!strcmp(kind, "hazard_take")) {
        char* p = dup_str(a[0]);
        int rc = F(cex_demo_hazard_take)(p, free);
        snprintf(res, res_len, "%d", rc);
    } else if (!strcmp(kind, "apply")) {
        int32_t v[64];
        int n = parse_csv_i32(a[0], v, 64);
        int32_t add = (int32_t)strtol(a[1], NULL, 10);
        snprintf(res, res_len, "%d", F(cex_demo_apply)(v, (size_t)n, cb_add, &add));
    } else if (!strcmp(kind, "call_on_thread")) {
        int32_t add = (int32_t)strtol(a[1], NULL, 10);
        snprintf(res, res_len, "%d", F(cex_demo_call_on_thread)(cb_add, &add, (int32_t)strtol(a[0], NULL, 10)));
    } else if (!strcmp(kind, "parse_exc")) {
        int32_t v = 0;
        const char* exc = F(cex_demo_parse_i32_exc)(a[0], &v);
        snprintf(res, res_len, "%d:%s", F(cex_demo_error_code)(exc), F(cex_demo_error_string)(exc));
    } else if (!strcmp(kind, "parse_status")) {
        int32_t v = 0;
        snprintf(res, res_len, "%d", F(cex_demo_parse_i32_status)(a[0], &v));
    } else if (!strcmp(kind, "parse_hybrid")) {
        int32_t v = 0;
        const char* exc = NULL;
        int code = F(cex_demo_parse_i32_hybrid)(a[0], &v, &exc);
        snprintf(res, res_len, "%d:%s", code, F(cex_demo_error_string)(exc));
    } else if (!strcmp(kind, "err_const")) {
        const char* c = err_const_by_name(a[0]);
        snprintf(res, res_len, "%s:%s", a[0], c ? c : "NULL");
    } else if (!strcmp(kind, "error_code")) {
        snprintf(res, res_len, "%d", F(cex_demo_error_code)(err_const_by_name(a[0])));
    } else if (!strcmp(kind, "strerror")) {
        int code = (int)strtol(a[0], NULL, 10);
        snprintf(res, res_len, "%d:%s", code, F(cex_demo_strerror)(code));
    } else if (!strcmp(kind, "traceback_contains")) {
        int32_t v = 0;
        F(cex_demo_parse_i32_exc)(a[0], &v);
        const char* tb = F(cex_demo_last_traceback)();
        snprintf(res, res_len, "%d", (tb && strstr(tb, "ArgumentError")) ? 1 : 0);
    } else {
        snprintf(res, res_len, "FAIL:unknown kind %s", kind);
    }
}

int
main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s cases.tsv [libpath]\n", argv[0]);
        return 2;
    }
#ifdef CEX_DEMO_DYNAMIC
    if (argc < 3) {
        fprintf(stderr, "dynamic runner needs a library path\n");
        return 2;
    }
    cex_demo_lib = OPEN_LIB(argv[2]);
    if (!cex_demo_lib) {
        fprintf(stderr, "cannot load %s\n", argv[2]);
        return 2;
    }
#endif

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 2;
    }

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (!line[0] || line[0] == '#') { continue; }
        char* fields[4] = { 0 };
        int nf = 0;
        char* p = line;
        while (nf < 4) {
            fields[nf++] = p;
            char* tab = strchr(p, '\t');
            if (!tab) { break; }
            *tab = '\0';
            p = tab + 1;
        }
        if (nf < 4) { continue; }
        char res[1024] = { 0 };
        run_case(fields[1], fields[2], res, sizeof(res));
        printf("%s\t%s\n", fields[0], res);
    }
    fclose(f);
    return 0;
}
