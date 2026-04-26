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
            "uacpi/source/default_handlers.c",
            "uacpi/source/event.c",
            "uacpi/source/interpreter.c",
            "uacpi/source/io.c",
            "uacpi/source/mutex.c",
            "uacpi/source/namespace.c",
            "uacpi/source/notify.c",
            "uacpi/source/opcodes.c",
            "uacpi/source/opregion.c",
            "uacpi/source/osi.c",
            "uacpi/source/registers.c",
            "uacpi/source/resources.c",
            "uacpi/source/shareable.c",
            "uacpi/source/stdlib.c",
            "uacpi/source/tables.c",
            "uacpi/source/types.c",
            "uacpi/source/uacpi.c",
            "uacpi/source/utilities.c",
        ])
        .includes(["uacpi/include/"])
        .define("UACPI_SIZED_FREES", None)
        .pic(false)
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
        .clang_args(["-I", "uacpi/include", "-ffreestanding"])
        .header("src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
