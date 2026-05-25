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
            format!("./uacpi/src/default_handlers.c"),
            format!("./uacpi/src/event.c"),
            format!("./uacpi/src/interpreter.c"),
            format!("./uacpi/src/io.c"),
            format!("./uacpi/src/mutex.c"),
            format!("./uacpi/src/namespace.c"),
            format!("./uacpi/src/notify.c"),
            format!("./uacpi/src/opcodes.c"),
            format!("./uacpi/src/opregion.c"),
            format!("./uacpi/src/osi.c"),
            format!("./uacpi/src/registers.c"),
            format!("./uacpi/src/resources.c"),
            format!("./uacpi/src/shareable.c"),
            format!("./uacpi/src/stdlib.c"),
            format!("./uacpi/src/tables.c"),
            format!("./uacpi/src/types.c"),
            format!("./uacpi/src/uacpi.c"),
            format!("./uacpi/src/utilities.c"),
        ])
        .includes(["./uacpi/include/"])
        .define("UACPI_SIZED_FREES", None)
        .pic(false)
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
        .clang_args(["-I", "./uacpi/include/", "-ffreestanding"])
        .header("./src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
