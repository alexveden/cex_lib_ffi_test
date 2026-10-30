use std::path::PathBuf;

fn main() {
    let lib_dir = PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").unwrap())
        .join("..")
        .join("..")
        .join("build")
        .join("lib");
    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let lib_name = if target_os == "windows" { "libcex_demo.lib" } else { "libcex_demo.a" };
    let lib_path = lib_dir.join(lib_name);

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    if target_os == "windows" {
        println!("cargo:rustc-link-lib=static:+verbatim={}", lib_name);
    } else {
        println!("cargo:rustc-link-lib=static=cex_demo");
    }
    if target_os == "linux" {
        println!("cargo:rustc-link-lib=dylib=pthread");
    }
    println!("cargo:rerun-if-changed={}", lib_path.display());
}
