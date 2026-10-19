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
            format!("./uACPI/source/default_handlers.c"),
            format!("./uACPI/source/event.c"),
            format!("./uACPI/source/interpreter.c"),
            format!("./uACPI/source/io.c"),
            format!("./uACPI/source/mutex.c"),
            format!("./uACPI/source/namespace.c"),
            format!("./uACPI/source/notify.c"),
            format!("./uACPI/source/opcodes.c"),
            format!("./uACPI/source/opregion.c"),
            format!("./uACPI/source/osi.c"),
            format!("./uACPI/source/registers.c"),
            format!("./uACPI/source/resources.c"),
            format!("./uACPI/source/shareable.c"),
            format!("./uACPI/source/sleep.c"),
            format!("./uACPI/source/stdlib.c"),
            format!("./uACPI/source/tables.c"),
            format!("./uACPI/source/types.c"),
            format!("./uACPI/source/uacpi.c"),
            format!("./uACPI/source/utilities.c"),
        ])
        .includes(["./uACPI/include/"])
        .define("UACPI_SIZED_FREES", None)
        .pic(false)
        .flag("-fno-stack-protector")
        .flag("-mcmodel=kernel")
        .flag("-fno-pic")
        .flag("-ffreestanding")
        .flag("-nostdlib")
        .flag("-mgeneral-regs-only")
        .flag("-mno-red-zone")
        .compile("uacpi");

    let bindings = bindgen::builder()
        .use_core()
        .wrap_unsafe_ops(true)
        .derive_default(true)
        .derive_debug(true)
        .prepend_enum_name(false)
        .clang_args(["-I", "./uACPI/include/", "-ffreestanding"])
        .header("./src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
