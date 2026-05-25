//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use std::{env, path::PathBuf};

use cc::Build;

fn main() {
    let mut build = Build::new();

    build
        .files([
            format!("./flanterm/src/flanterm.c"),
            format!("./flanterm/src/flanterm_backends/fb.c"),
        ])
        .includes(["./flanterm/src/"])
        .pic(false)
        .flag("-mcmodel=kernel")
        .flag("-fno-pic")
        .flag("-ffreestanding")
        .flag("-nostdlib")
        .flag("-mgeneral-regs-only")
        .flag("-mno-red-zone")
        .compile("flanterm");

    let bindings = bindgen::builder()
        .use_core()
        .wrap_unsafe_ops(true)
        .derive_default(true)
        .derive_debug(true)
        .prepend_enum_name(false)
        .clang_args(["-I", "./flanterm/src/", "-ffreestanding"])
        .header("./src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
