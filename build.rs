use std::env;
use std::process::Command;

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let xml = format!("{}/resources.gresource.xml", manifest_dir);
    let out_dir = env::var("OUT_DIR").unwrap();
    let target = format!("{}/resources.gresource", out_dir);

    // Compile gresource bundle
    let status = Command::new("glib-compile-resources")
        .args([
            "--target",
            &target,
            "--sourcedir",
            &manifest_dir,
            "--generate",
            &xml,
        ])
        .status();

    match status {
        Ok(s) if s.success() => {
            println!("cargo:rerun-if-changed={}", xml);
            println!("cargo:rerun-if-changed=assets/logo.png");
        }
        _ => {
            // glib-compile-resources not available — warn but don't fail
            println!("cargo:warning=glib-compile-resources not found; icons will not be embedded");
            println!("cargo:warning=Install glib2-devel to enable icon embedding");
        }
    }
}
