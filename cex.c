#if !defined(cex$enable_minimal)

#if __has_include("cex_config.h")
// Custom config file
#    include "cex_config.h"
#else
// Overriding config values
#    define cexy$cc_include "-I.", "-I./lib"
#    define CEX_LOG_LVL 4 /* 0 (mute all) - 5 (log$trace) */
#endif

#define CEX_IMPLEMENTATION
#define CEX_BUILD
#include "cex.h"

Exception cmd_build_lib(int argc, char** argv, void* user_ctx);

int
main(int argc, char** argv)
{

    cexy$initialize(); // cex self rebuild and init
    argparse_c args = {
        .description = cexy$description,
        .epilog = cexy$epilog,
        .usage = cexy$usage,
        argparse$cmd_list(
            cexy$cmd_all,
            cexy$cmd_fuzz, /* feel free to make your own if needed */
            cexy$cmd_test, /* feel free to make your own if needed */
            cexy$cmd_app,  /* feel free to make your own if needed */
            { .name = "build-lib", .func = cmd_build_lib, .help = "Custom build command" },
        ),
    };
    if (argparse.parse(&args, argc, argv)) { return 1; }
    void* my_user_ctx = NULL; // passed as `user_ctx` to command
    e$except (err, argparse.run_command(&args, my_user_ctx)) {
        if (err != Error.argsparse) { e$traceback_print(stderr); }
        return 1;
    }
    return 0;
}

typedef struct
{
    char* triple;
    char* ext;
    bool is_macos;
    bool is_windows;
} cex_build_lib__target_s;

static const cex_build_lib__target_s cex_build_lib__cross_targets[] = {
    { "x86_64-windows-gnu", ".dll", false, true },
    { "aarch64-macos", ".dylib", true, false },
    { "x86_64-macos", ".dylib", true, false },
    { "aarch64-linux-gnu", ".so", false, false },
};

static Exception
cex_build_lib__native(char* out_dir)
{
    char* obj = str.fmt(mem$, "%s/cex_demo.o", out_dir);
    e$ret(os$cmd(
        cexy$cc,
        "-c",
        "lib/cex_demo.c",
        "-o",
        obj,
        "-g3",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-fPIC",
        "-fvisibility=hidden",
        "-I.",
        "-I./lib"
    ));

    char* stat_lib = str.fmt(mem$, "%s/libcex_demo%s", out_dir, cexy$build_ext_lib_stat);
    e$ret(os$cmd("ar", "rcs", stat_lib, obj));

#if defined(__APPLE__)
    char* dyn_lib = str.fmt(mem$, "%s/libcex_demo.dylib", out_dir);
#else
    char* dyn_lib = str.fmt(mem$, "%s/libcex_demo%s", out_dir, cexy$build_ext_lib_dyn);
#endif
    arr$(char*) args = arr$new(args, mem$);
    arr$pushm(args, cexy$cc);
#if defined(_WIN32)
    arr$pushm(args, "-shared");
#elif defined(__APPLE__)
    arr$pushm(args, "-dynamiclib", "-pthread");
#else
    arr$pushm(args, "-shared", "-Wl,-soname,libcex_demo.so", "-pthread");
#endif
    arr$pushm(args, "-fPIC", "-fvisibility=hidden", "-o", dyn_lib, obj);
    arr$push(args, NULL);
    e$ret(os$cmda(args));
    arr$free(args);

    log$info("built: %s\n", stat_lib);
    log$info("built: %s\n", dyn_lib);
    return EOK;
}

static Exception
cex_build_lib__cross_one(char* out_dir, const cex_build_lib__target_s* t)
{
    char* dir = str.fmt(mem$, "%s/%s", out_dir, t->triple);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/x", dir)));
    char* out = str.fmt(mem$, "%s/libcex_demo%s", dir, t->ext);

    arr$(char*) args = arr$new(args, mem$);
    arr$pushm(args, "zig", "cc", "-target", t->triple);
    arr$pushm(args, "-fPIC", "-fvisibility=hidden");
    arr$pushm(args, t->is_macos ? "-dynamiclib" : "-shared");
    if (!t->is_windows) { arr$pushm(args, "-pthread"); }
    arr$pushm(args, "-I.", "-I./lib", "lib/cex_demo.c", "-o", out);
    arr$push(args, NULL);
    e$ret(os$cmda(args));
    arr$free(args);

    log$info("built: %s\n", out);
    return EOK;
}

static Exception
cex_build_lib__cross(char* out_dir)
{
    for (usize i = 0; i < sizeof(cex_build_lib__cross_targets) / sizeof(cex_build_lib__cross_targets[0]); i++) {
        e$ret(cex_build_lib__cross_one(out_dir, &cex_build_lib__cross_targets[i]));
    }
    return EOK;
}

/// Build static + shared CEX demo library (native and cross targets)
Exception
cmd_build_lib(int argc, char** argv, void* user_ctx)
{
    (void)user_ctx;
    char* target = "native";
    for (int i = 1; i < argc; i++) {
        if (str.starts_with(argv[i], "--target=")) { target = argv[i] + 9; }
    }

    bool want_native = str.eq(target, "native") || str.eq(target, "all");
    bool want_cross = str.eq(target, "cross") || str.eq(target, "all");
    if (!want_native && !want_cross) {
        log$error("unknown --target=%s (native|cross|all)\n", target);
        return e$raise(Error.argument, "unknown target");
    }

    char* out_dir = cexy$build_dir "/lib";
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/x", out_dir)));
    if (want_native) { e$ret(cex_build_lib__native(out_dir)); }
    if (want_cross) { e$ret(cex_build_lib__cross(out_dir)); }
    return EOK;
}

#endif
