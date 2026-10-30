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

    println!("cargo:rustc-link-arg={}", lib_path.display());
    if target_os == "linux" {
        println!("cargo:rustc-link-arg=-lpthread");
    }
    println!("cargo:rerun-if-changed={}", lib_path.display());
}
