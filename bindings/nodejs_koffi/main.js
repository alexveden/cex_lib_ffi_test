const koffi = require("koffi");

const Request = koffi.struct("cex_demo_request_t", { value: "int32_t" });

function library_path() {
    if (process.platform === "darwin") return "build/lib/libcex_demo.dylib";
    if (process.platform === "win32") return "build/lib/libcex_demo.dll";
    return "build/lib/libcex_demo.so";
}

const lib = koffi.load(library_path());
const run_raw = lib.func("void *cex_demo_run(cex_demo_request_t *req)");
const traceback_raw = lib.func("void *cex_demo_traceback(char *buf, size_t size)");

// koffi has no API for exported data symbols. cex_demo_Error.ok is NULL and
// cex_demo_run(NULL) returns the same address as cex_demo_Error.argument.
const argument_error = run_raw(null);

function error_text(exc) {
    return koffi.decode(exc, "char", -1);
}

function buffer_text(buf) {
    const end = buf.indexOf(0);
    return buf.toString("utf8", 0, end === -1 ? buf.length : end);
}

function traceback() {
    const buf = Buffer.alloc(512);
    const exc = traceback_raw(buf, buf.length);
    if (exc !== null) throw new Error(error_text(exc));
    return buffer_text(buf);
}

function run(value) {
    if (value === null) throw new Error(error_text(argument_error));
    const req = koffi.alloc(Request, 1);
    try {
        koffi.encode(req, Request, { value });
        const exc = run_raw(req);
        if (exc === argument_error) throw new Error(error_text(exc));
        if (exc !== null) throw new Error(error_text(exc));
        return koffi.decode(req, Request).value;
    } finally {
        koffi.free(req);
    }
}

function main() {
    const req = koffi.alloc(Request, 1);
    koffi.encode(req, Request, { value: 41 });
    const exc = run_raw(req);
    const value = koffi.decode(req, Request).value;
    koffi.free(req);
    if (exc !== null || value !== 42) {
        console.error(`run failed: exc=${exc === null ? "NULL" : error_text(exc)} value=${value}`);
        return 1;
    }
    console.log(`value=${value}`);

    if (run(41) !== 42) {
        console.error("wrapper failed");
        return 1;
    }

    const null_exc = run_raw(null);
    if (null_exc !== argument_error) {
        console.error("NULL request returned unexpected error");
        return 1;
    }
    console.log(`error=${error_text(null_exc)}`);

    if (traceback_raw(null, 0) !== argument_error) {
        console.error("NULL traceback buffer returned unexpected error");
        return 1;
    }
    const overflow_exc = traceback_raw(Buffer.alloc(1), 1);
    if (overflow_exc === null || overflow_exc === argument_error) {
        console.error("small traceback buffer returned unexpected error");
        return 1;
    }
    if (traceback_raw(Buffer.alloc(1), 1) !== overflow_exc) {
        console.error("small traceback buffer returned inconsistent error");
        return 1;
    }
    const tb = traceback();
    if (!tb.includes("cex_demo__validate_request") || !tb.includes("cex_demo_run")) {
        console.error("traceback missing frames");
        return 1;
    }
    process.stdout.write(`traceback=${tb}`);

    try {
        run(null);
    } catch {
        return 0;
    }
    console.error("wrapper NULL request unexpectedly succeeded");
    return 1;
}

process.exitCode = main();
