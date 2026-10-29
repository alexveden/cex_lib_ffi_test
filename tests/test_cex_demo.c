#define CEX_IMPLEMENTATION
#define CEX_TEST
#include "cex.h"
#include "lib/cex_demo.c"

static int32_t
cb_add(int32_t value, void* user_data)
{
    return value + *(int32_t*)user_data;
}

test$case(cex_demo_primitives)
{
    tassert_eq(cex_demo_id_i8(-5), -5);
    tassert_eq(cex_demo_id_u64(UINT64_MAX), UINT64_MAX);
    tassert_eq_almost(cex_demo_mix(1.5, 2.0f, -1, 2), 4.5, 1e-9);
    tassert(cex_demo_is_odd(3));
    tassert(!cex_demo_is_odd(4));
    return EOK;
}

test$case(cex_demo_strings)
{
    char buf[16] = { 0 };
    char small[4] = { 0 };

    tassert_eq(cex_demo_str_len("hello"), 5);
    tassert_eq(cex_demo_str_len(NULL), 0);
    tassert_eq(cex_demo_str_concat_need("foo", "bar"), 7);
    tassert_eq(cex_demo_str_concat(buf, sizeof(buf), "foo", "bar"), 0);
    tassert_eq(buf, "foobar");
    tassert_eq(cex_demo_str_concat(small, sizeof(small), "foo", "bar"), -1);
    tassert_eq(small, "");
    return EOK;
}

test$case(cex_demo_arrays)
{
    int32_t nums[] = { 1, 2, 3 };
    tassert_eq(cex_demo_sum_i32(nums, 3), 6);
    tassert_eq(cex_demo_sum_i32(NULL, 0), 0);
    cex_demo_scale_i32(nums, 3, 2);
    tassert_eq(nums[0], 2);
    tassert_eq(nums[1], 4);
    tassert_eq(nums[2], 6);
    return EOK;
}

test$case(cex_demo_structs)
{
    cex_demo_point_t p = cex_demo_point_add((cex_demo_point_t){ 1, 2 }, (cex_demo_point_t){ 3, 4 });
    tassert_eq(p.x, 4);
    tassert_eq(p.y, 6);
    cex_demo_point_scale(&p, 3);
    tassert_eq(p.x, 12);
    tassert_eq(p.y, 18);

    cex_demo_padded_t padded = cex_demo_padded_make(7, 1234567890123LL, 2.5, "abcdefghijklmnopq");
    tassert_eq(padded.tag, 7);
    tassert_eq(padded.big, 1234567890123LL);
    tassert_eq_almost(padded.d, 2.5, 1e-9);
    tassert_eq(padded.name, "abcdefghijklmno");
    tassert_eq(cex_demo_sizeof_padded(), 40);
    return EOK;
}

test$case(cex_demo_enum_out_params)
{
    tassert_eq(cex_demo_mode_next(CEX_DEMO_MODE_A), CEX_DEMO_MODE_B);
    tassert_eq(cex_demo_mode_next(CEX_DEMO_MODE_B), CEX_DEMO_MODE_C);
    tassert_eq(cex_demo_mode_next(CEX_DEMO_MODE_C), CEX_DEMO_MODE_A);

    int32_t q = 0;
    int32_t r = 0;
    tassert_eq(cex_demo_divmod(7, 2, &q, &r), 0);
    tassert_eq(q, 3);
    tassert_eq(r, 1);
    tassert_eq(cex_demo_divmod(7, 0, &q, &r), -1);
    return EOK;
}

test$case(cex_demo_counter)
{
    cex_demo_counter_t* c = cex_demo_counter_new(10);
    tassert(c != NULL);
    tassert_eq(cex_demo_counter_next(c), 10);
    tassert_eq(cex_demo_counter_next(c), 11);
    cex_demo_counter_free(c);
    return EOK;
}

test$case(cex_demo_ownership)
{
    char* a = cex_demo_str_dup("hello");
    tassert_eq(a, "hello");
    cex_demo_free(a);

    char* b = cex_demo_hazard_lib_alloc("world");
    tassert_eq(b, "world");
    cex_demo_free(b);

    char* c = cex_demo_str_dup_with("caller", malloc, free);
    tassert_eq(c, "caller");
    free(c);

    char* d = malloc(4);
    memcpy(d, "xyz", 4);
    tassert_eq(cex_demo_hazard_take(d, free), 0);
    tassert_eq(cex_demo_hazard_take(NULL, free), -1);
    return EOK;
}

test$case(cex_demo_callbacks)
{
    int32_t nums[] = { 1, 2, 3 };
    int32_t add = 10;
    tassert_eq(cex_demo_apply(nums, 3, cb_add, &add), 36);
    tassert_eq(cex_demo_call_on_thread(cb_add, &add, 5), 15);
    return EOK;
}

test$case(cex_demo_errors)
{
    int32_t v = 0;
    const char* exc = NULL;

    tassert(cex_demo_parse_i32_exc("42", &v) == NULL);
    tassert_eq(v, 42);
    tassert(cex_demo_parse_i32_exc("abc", &v) == cex_demo_Err_argument);
    tassert(cex_demo_parse_i32_exc("2147483648", &v) == cex_demo_Err_overflow);
    tassert_eq(cex_demo_parse_i32_status("abc", &v), CEX_DEMO_ERR_ARGUMENT);
    tassert_eq(cex_demo_parse_i32_status("42", &v), CEX_DEMO_ERR_OK);
    tassert_eq(cex_demo_parse_i32_hybrid("abc", &v, &exc), CEX_DEMO_ERR_ARGUMENT);
    tassert(exc == cex_demo_Err_argument);

    tassert_eq(cex_demo_error_code(NULL), CEX_DEMO_ERR_OK);
    tassert_eq(cex_demo_error_code(cex_demo_Err_overflow), CEX_DEMO_ERR_OVERFLOW);
    tassert_eq(cex_demo_error_code("custom"), CEX_DEMO_ERR_UNKNOWN);
    tassert_eq((char*)cex_demo_error_string(NULL), "OK");
    tassert_eq((char*)cex_demo_error_string(cex_demo_Err_argument), "ArgumentError");
    tassert_eq((char*)cex_demo_strerror(CEX_DEMO_ERR_OVERFLOW), "OverflowError");
    tassert_eq((char*)cex_demo_strerror(123), "UnknownError");

    tassert(cex_demo_last_traceback() != NULL);
    tassert(str.find((char*)cex_demo_last_traceback(), "ArgumentError") != NULL);
    return EOK;
}

test$case(cex_demo_version)
{
    cex_demo_version_t v = cex_demo_version();
    tassert_eq(v.major, 1);
    tassert_eq(v.minor, 0);
    tassert_eq(v.patch, 0);
    tassert_eq(v.struct_size, sizeof(cex_demo_version_t));
    return EOK;
}

test$main();
