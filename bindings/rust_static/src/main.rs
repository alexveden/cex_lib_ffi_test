use std::ffi::{c_char, CStr};

#[repr(C)]
struct CexDemoRequest {
    value: i32,
}

#[repr(C)]
struct CexDemoErrors {
    ok: *const c_char,
    argument: *const c_char,
    #[allow(dead_code)]
    overflow: *const c_char,
}

extern "C" {
    fn cex_demo_run(req: *mut CexDemoRequest) -> *const c_char;
    fn cex_demo_traceback(buf: *mut c_char, size: usize) -> *const c_char;
    #[allow(non_upper_case_globals)]
    static cex_demo_Error: CexDemoErrors;
}

fn error_text(exc: *const c_char) -> String {
    unsafe { CStr::from_ptr(exc) }.to_string_lossy().into_owned()
}

fn is_ok(exc: *const c_char) -> bool {
    std::ptr::eq(exc, unsafe { cex_demo_Error.ok })
}

fn is_argument(exc: *const c_char) -> bool {
    std::ptr::eq(exc, unsafe { cex_demo_Error.argument })
}

fn traceback() -> Result<String, String> {
    let mut buf = [0u8; 512];
    let exc = unsafe { cex_demo_traceback(buf.as_mut_ptr() as *mut c_char, buf.len()) };
    if !is_ok(exc) {
        return Err(error_text(exc));
    }
    Ok(unsafe { CStr::from_ptr(buf.as_ptr() as *const c_char) }
        .to_string_lossy()
        .into_owned())
}

fn main() {
    let mut req = CexDemoRequest { value: 41 };
    let exc = unsafe { cex_demo_run(&mut req) };
    if !is_ok(exc) || req.value != 42 {
        let text = if exc.is_null() {
            String::from("NULL")
        } else {
            error_text(exc)
        };
        eprintln!("run failed: exc={} value={}", text, req.value);
        std::process::exit(1);
    }
    println!("value={}", req.value);

    let null_exc = unsafe { cex_demo_run(std::ptr::null_mut()) };
    if !is_argument(null_exc) {
        eprintln!("NULL request returned unexpected error");
        std::process::exit(1);
    }
    println!("error={}", error_text(null_exc));

    let tb = match traceback() {
        Ok(tb) => tb,
        Err(err) => {
            eprintln!("traceback failed: {}", err);
            std::process::exit(1);
        }
    };
    if !tb.contains("cex_demo__validate_request") || !tb.contains("cex_demo_run") {
        eprintln!("traceback missing frames");
        std::process::exit(1);
    }
    print!("traceback={}", tb);
}
