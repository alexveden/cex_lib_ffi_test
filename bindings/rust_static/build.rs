use std::path::PathBuf;

fn main() {
    let lib_dir = PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").unwrap())
        .join("..")
        .join("..")
        .join("build")
        .join("lib");

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=cex_demo");
    if std::env::var("CARGO_CFG_TARGET_OS").as_deref() == Ok("linux") {
        println!("cargo:rustc-link-lib=dylib=pthread");
    }
    println!(
        "cargo:rerun-if-changed={}",
        lib_dir.join("libcex_demo.a").display()
    );
}
