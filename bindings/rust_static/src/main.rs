use std::ffi::{c_char, CStr};

#[repr(C)]
struct CexDemoRequest {
    value: i32,
}

#[repr(C)]
struct CexDemoErrors {
    ok: *const c_char,
    argument: *const c_char,
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

fn is_overflow(exc: *const c_char) -> bool {
    std::ptr::eq(exc, unsafe { cex_demo_Error.overflow })
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

fn run(value: Option<i32>) -> Result<i32, String> {
    let mut req = CexDemoRequest {
        value: value.unwrap_or_default(),
    };
    let req_ptr = match value {
        Some(_) => &mut req as *mut CexDemoRequest,
        None => std::ptr::null_mut(),
    };
    let exc = unsafe { cex_demo_run(req_ptr) };
    if is_argument(exc) {
        return Err(error_text(exc));
    }
    if !is_ok(exc) {
        return Err(error_text(exc));
    }
    Ok(req.value)
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

    if run(Some(41)) != Ok(42) {
        eprintln!("wrapper failed");
        std::process::exit(1);
    }

    let null_exc = unsafe { cex_demo_run(std::ptr::null_mut()) };
    if !is_argument(null_exc) {
        eprintln!("NULL request returned unexpected error");
        std::process::exit(1);
    }
    println!("error={}", error_text(null_exc));

    if !is_argument(unsafe { cex_demo_traceback(std::ptr::null_mut(), 0) }) {
        eprintln!("NULL traceback buffer returned unexpected error");
        std::process::exit(1);
    }
    let mut tiny = [0u8; 8];
    if !is_overflow(unsafe {
        cex_demo_traceback(tiny.as_mut_ptr() as *mut c_char, tiny.len())
    }) {
        eprintln!("small traceback buffer returned unexpected error");
        std::process::exit(1);
    }
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

    if run(None).is_err() {
        return;
    }
    eprintln!("wrapper NULL request unexpectedly succeeded");
    std::process::exit(1);
}
