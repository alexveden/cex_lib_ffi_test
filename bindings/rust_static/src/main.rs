use std::ffi::{c_char, CStr};

#[repr(C)]
struct CexDemoRequest {
    value: i32,
}

#[repr(C)]
struct CexDemoErrors {
    ok: *const c_char,
    argument: *const c_char,
}

extern "C" {
    fn cex_demo_run(req: *mut CexDemoRequest) -> *const c_char;
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

    if run(None).is_err() {
        return;
    }
    eprintln!("wrapper NULL request unexpectedly succeeded");
    std::process::exit(1);
}
