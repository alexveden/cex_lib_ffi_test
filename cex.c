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
Exception cmd_bindings(int argc, char** argv, void* user_ctx);

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
            { .name = "bindings",
              .func = cmd_bindings,
              .help = "Build and run language bindings (all|<name>...)" },
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
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", dir)));
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
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    if (want_native) { e$ret(cex_build_lib__native(out_dir)); }
    if (want_cross) { e$ret(cex_build_lib__cross(out_dir)); }
    return EOK;
}

typedef struct
{
    char* name;
    Exception (*run)(void);
} binding_s;

static Exception
bindings__run_c_static(void)
{
    char* out_dir = str.fmt(mem$, "%s/bindings/c_static", cexy$build_dir);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    char* obj = str.fmt(mem$, "%s/main.o", out_dir);
    char* exe = str.fmt(mem$, "%s/main%s", out_dir, cexy$build_ext_exe);
    char* lib = str.fmt(mem$, "%s/lib/libcex_demo%s", cexy$build_dir, cexy$build_ext_lib_stat);
    e$ret(os$cmd(
        cexy$cc,
        "-Wall",
        "-Wextra",
        "-Werror",
        "-DCEX_DEMO_STATIC",
        "-Ilib",
        "-c",
        "bindings/c_static/main.c",
        "-o",
        obj
    ));
    e$ret(os$cmd(cexy$cc, obj, lib, "-o", exe));
    return os$cmd(exe);
}

static Exception
bindings__run_cpp_static(void)
{
    char* out_dir = str.fmt(mem$, "%s/bindings/cpp_static", cexy$build_dir);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    char* obj = str.fmt(mem$, "%s/main.o", out_dir);
    char* exe = str.fmt(mem$, "%s/main%s", out_dir, cexy$build_ext_exe);
    char* lib = str.fmt(mem$, "%s/lib/libcex_demo%s", cexy$build_dir, cexy$build_ext_lib_stat);
    e$ret(os$cmd(
        "g++",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-std=c++20",
        "-DCEX_DEMO_STATIC",
        "-Ilib",
        "-c",
        "bindings/cpp_static/main.cpp",
        "-o",
        obj
    ));
    e$ret(os$cmd("g++", obj, lib, "-o", exe));
    return os$cmd(exe);
}

static Exception
bindings__run_go_cgo(void)
{
    if (!os.cmd.exists("go")) {
        log$error("go toolchain not found\n");
        return e$raise(Error.not_found, "go toolchain not found");
    }
    char* out_dir = str.fmt(mem$, "%s/bindings/go_cgo", cexy$build_dir);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    char* exe = os.path.absolute(str.fmt(mem$, "%s/main%s", out_dir, cexy$build_ext_exe), mem$);
    e$ret(os.env.set("CGO_ENABLED", "1"));
    e$ret(os$cmd("go", "build", "-C", "bindings/go_cgo", "-o", exe, "."));
    return os$cmd(exe);
}

static Exception
bindings__run_nodejs_koffi(void)
{
    if (!os.cmd.exists("node")) {
        log$error("node not found\n");
        return e$raise(Error.not_found, "node not found");
    }
    char* node_dir = str.fmt(mem$, "%s/node", cexy$build_dir);
    char* node_modules = os.path.absolute(str.fmt(mem$, "%s/node_modules", node_dir), mem$);
    if (!os.path.exists(str.fmt(mem$, "%s/koffi", node_modules))) {
#if defined(_WIN32)
        e$ret(os$cmd(
            "cmd.exe",
            "/c",
            "npm",
            "install",
            "--prefix",
            node_dir,
            "--no-audit",
            "--no-fund",
            "--silent",
            "koffi"
        ));
#else
        e$ret(os$cmd(
            "npm",
            "install",
            "--prefix",
            node_dir,
            "--no-audit",
            "--no-fund",
            "--silent",
            "koffi"
        ));
#endif
    }
    e$ret(os.env.set("NODE_PATH", node_modules));
    return os$cmd("node", "bindings/nodejs_koffi/main.js");
}

static char*
bindings__capture(char** args, usize args_len)
{
    os_cmd_c cmd = { 0 };
    e$except (err, os.cmd.create(&cmd, args, args_len, NULL)) { return NULL; }
    char* out = os.cmd.read_all(&cmd, mem$);
    e$except (err, os.cmd.wait(&cmd, 1, 0)) { return NULL; }
    if (!out) { return NULL; }
    return str.fmt(mem$, "%S", str.slice.strip(str.sstr(out)));
}

static char*
bindings__python_exe(void)
{
    if (os.cmd.exists("python3")) { return "python3"; }
    if (os.cmd.exists("python")) { return "python"; }
    return NULL;
}

static Exception
bindings__ensure_venv(char** py)
{
    char* python = bindings__python_exe();
    if (!python) {
        log$error("python not found\n");
        return e$raise(Error.not_found, "python not found");
    }
    char* venv = str.fmt(mem$, "%s/venv", cexy$build_dir);
    char* py_posix = str.fmt(mem$, "%s/bin/python", venv);
    char* py_win = str.fmt(mem$, "%s/Scripts/python.exe", venv);
    if (!os.path.exists(py_posix) && !os.path.exists(py_win)) {
#if defined(_WIN32)
        e$ret(os$cmd(python, "-m", "venv", "--without-pip", "--system-site-packages", venv));
#else
        e$ret(os$cmd(python, "-m", "venv", venv));
#endif
    }
    *py = os.path.exists(py_win) ? py_win : py_posix;
    if (!os.path.exists(*py)) {
        log$error("python venv not found\n");
        return e$raise(Error.not_found, "python venv not found");
    }
    return EOK;
}

static Exception
bindings__run_python_capi(void)
{
    char* py = bindings__python_exe();
    if (!py) {
        log$error("python not found\n");
        return e$raise(Error.not_found, "python not found");
    }
    char* include = bindings__capture(
        (char*[]){ py, "-c", "import sysconfig;print(sysconfig.get_paths()['include'])", NULL },
        4
    );
    char* suffix = bindings__capture(
        (char*[]){ py, "-c", "import sysconfig;print(sysconfig.get_config_var('EXT_SUFFIX'))", NULL },
        4
    );
    if (!include || !*include || !suffix || !*suffix) {
        log$error("python sysconfig query failed\n");
        return e$raise(Error.not_found, "python sysconfig query failed");
    }
    char* out_dir = str.fmt(mem$, "%s/bindings/python_capi", cexy$build_dir);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    char* ext = str.fmt(mem$, "%s/_cex_demo_capi%s", out_dir, suffix);
    char* lib = str.fmt(mem$, "%s/lib/libcex_demo%s", cexy$build_dir, cexy$build_ext_lib_stat);

    arr$(char*) args = arr$new(args, mem$);
    arr$pushm(args, cexy$cc, "-Wall", "-Wextra", "-Werror", "-shared", "-fPIC", "-Ilib", "-DCEX_DEMO_STATIC");
    arr$pushm(args, str.fmt(mem$, "-I%s", include));
#if defined(__APPLE__)
    arr$pushm(args, "-undefined", "dynamic_lookup");
#elif defined(_WIN32)
    char* pylib = bindings__capture(
        (char*[]){ py,
                   "-c",
                   "import sys,sysconfig,os\n"
                   "v=sysconfig.get_config_var('LDVERSION') or '%d.%d'%sys.version_info[:2]\n"
                   "l=sysconfig.get_config_var('LDLIBRARY')\n"
                   "c=[os.path.join(sysconfig.get_config_var('LIBDIR') or '',l) if l else '',"
                   "os.path.join(sys.base_prefix,'lib','libpython%s.dll.a'%v),"
                   "os.path.join(sys.base_prefix,'lib','libpython%s.a'%v),"
                   "os.path.join(sys.base_prefix,'libs','python%s.lib'%v)]\n"
                   "print(next((p for p in c if p and os.path.exists(p)),''))",
                   NULL },
        4
    );
    if (!pylib || !*pylib || !os.path.exists(pylib)) {
        log$error("python import library not found\n");
        return e$raise(Error.not_found, "python import library not found");
    }
    arr$pushm(args, pylib);
#endif
    arr$pushm(args, "bindings/python_capi/main.c", lib, "-o", ext);
    arr$push(args, NULL);
    e$ret(os$cmda(args));
    arr$free(args);
    return os$cmd(py, "bindings/python_capi/main.py");
}

static Exception
bindings__run_python_cffi(void)
{
    char* py = NULL;
    e$ret(bindings__ensure_venv(&py));
    Exc cffi = os$cmd(
        py,
        "-c",
        "import importlib.util,sys; sys.exit(importlib.util.find_spec('cffi') is None)"
    );
    if (cffi != EOK) { e$ret(os$cmd(py, "-m", "pip", "install", "--quiet", "cffi", "setuptools")); }
    e$ret(os$cmd(py, "bindings/python_cffi/build.py"));
    return os$cmd(py, "bindings/python_cffi/main.py");
}

static Exception
bindings__run_python_ctypes(void)
{
    char* py = bindings__python_exe();
    if (!py) {
        log$error("python not found\n");
        return e$raise(Error.not_found, "python not found");
    }
    return os$cmd(py, "bindings/python_ctypes/main.py");
}

static Exception
bindings__run_python_cython(void)
{
    char* py = bindings__python_exe();
    if (!py) {
        log$error("python not found\n");
        return e$raise(Error.not_found, "python not found");
    }
    e$ret(bindings__ensure_venv(&py));
    if (os$cmd(py, "-c", "import Cython") != EOK) {
        e$ret(os$cmd(py, "-m", "pip", "install", "--quiet", "cython", "setuptools"));
    }
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/bindings/python_cython/", cexy$build_dir)));
    return os$cmd(py, "bindings/python_cython/main.py");
}

static Exception
bindings__run_rust_static(void)
{
    if (!os.cmd.exists("cargo")) {
        log$error("cargo toolchain not found\n");
        return e$raise(Error.not_found, "cargo toolchain not found");
    }
    char* target_dir = os.path.absolute(str.fmt(mem$, "%s/rust", cexy$build_dir), mem$);
    e$ret(os.env.set("CARGO_TARGET_DIR", target_dir));
    e$ret(os$cmd(
        "cargo",
        "build",
        "--quiet",
        "--offline",
        "--manifest-path",
        "bindings/rust_static/Cargo.toml"
    ));
    char* exe = str.fmt(mem$, "%s/debug/cex_demo_rust_static%s", target_dir, cexy$build_ext_exe);
    return os$cmd(exe);
}

static Exception
bindings__run_zig_static(void)
{
    if (!os.cmd.exists("zig")) {
        log$error("zig toolchain not found\n");
        return e$raise(Error.not_found, "zig toolchain not found");
    }
    char* out_dir = str.fmt(mem$, "%s/bindings/zig_static", cexy$build_dir);
    e$ret(os.fs.mkpath(str.fmt(mem$, "%s/", out_dir)));
    char* exe = str.fmt(mem$, "%s/main%s", out_dir, cexy$build_ext_exe);
    char* lib = str.fmt(mem$, "%s/lib/libcex_demo%s", cexy$build_dir, cexy$build_ext_lib_stat);
    e$ret(os$cmd(
        "zig",
        "build-exe",
        "bindings/zig_static/main.zig",
        "-lc",
        "-Ilib",
        lib,
        str.fmt(mem$, "-femit-bin=%s", exe)
    ));
    return os$cmd(exe);
}

static binding_s bindings__table[] = {
    { "c_static", bindings__run_c_static },
    { "cpp_static", bindings__run_cpp_static },
    { "go_cgo", bindings__run_go_cgo },
    { "nodejs_koffi", bindings__run_nodejs_koffi },
    { "python_capi", bindings__run_python_capi },
    { "python_cffi", bindings__run_python_cffi },
    { "python_ctypes", bindings__run_python_ctypes },
    { "python_cython", bindings__run_python_cython },
    { "rust_static", bindings__run_rust_static },
    { "zig_static", bindings__run_zig_static },
};

static Exception
bindings__run_entry(binding_s* binding, u32* passed, u32* failed)
{
    io.printf("== %s ==\n", binding->name);
    e$except (err, binding->run()) {
        io.printf("FAIL %s\n", binding->name);
        (*failed)++;
        return err;
    }
    io.printf("PASS %s\n", binding->name);
    (*passed)++;
    return EOK;
}

Exception
cmd_bindings(int argc, char** argv, void* user_ctx)
{
    (void)user_ctx;
    bool run_all = argc < 2;
    for (int i = 1; i < argc; i++) {
        if (str.eq(argv[i], "all")) {
            run_all = true;
            continue;
        }
        bool found = false;
        for$eachp (b, bindings__table, arr$len(bindings__table)) {
            if (str.eq(b->name, argv[i])) {
                found = true;
                break;
            }
        }
        if (found) { continue; }
        log$error("unknown binding: %s\n", argv[i]);
        io.printf("available:");
        for$eachp (b, bindings__table, arr$len(bindings__table)) { io.printf(" %s", b->name); }
        io.printf("\n");
        return e$raise(Error.argument, "unknown binding");
    }

    char* build_argv[] = { "build-lib", NULL };
    e$ret(cmd_build_lib(1, build_argv, NULL));

    u32 passed = 0;
    u32 failed = 0;
    if (run_all) {
        for$eachp (b, bindings__table, arr$len(bindings__table)) {
            e$except (err, bindings__run_entry(b, &passed, &failed)) {}
        }
    } else {
        for (int i = 1; i < argc; i++) {
            for$eachp (b, bindings__table, arr$len(bindings__table)) {
                if (!str.eq(b->name, argv[i])) { continue; }
                e$except (err, bindings__run_entry(b, &passed, &failed)) {}
                break;
            }
        }
    }
    io.printf("bindings: passed=%u failed=%u\n", passed, failed);
    if (failed) { return e$raise(Error.runtime, "bindings failed"); }
    return EOK;
}

#endif
