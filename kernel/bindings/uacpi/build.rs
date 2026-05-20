//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use std::{env, path::PathBuf};

use cc::Build;

fn main() {
    let sysroot = std::env::var("SYSROOT").unwrap_or_else(|_| "/sysroot".to_string());

    let include_path = format!("{sysroot}/usr/include/uacpi/");

    let mut build = Build::new();

    build
        .files([
            format!("{sysroot}/usr/src/uacpi/default_handlers.c"),
            format!("{sysroot}/usr/src/uacpi/event.c"),
            format!("{sysroot}/usr/src/uacpi/interpreter.c"),
            format!("{sysroot}/usr/src/uacpi/io.c"),
            format!("{sysroot}/usr/src/uacpi/mutex.c"),
            format!("{sysroot}/usr/src/uacpi/namespace.c"),
            format!("{sysroot}/usr/src/uacpi/notify.c"),
            format!("{sysroot}/usr/src/uacpi/opcodes.c"),
            format!("{sysroot}/usr/src/uacpi/opregion.c"),
            format!("{sysroot}/usr/src/uacpi/osi.c"),
            format!("{sysroot}/usr/src/uacpi/registers.c"),
            format!("{sysroot}/usr/src/uacpi/resources.c"),
            format!("{sysroot}/usr/src/uacpi/shareable.c"),
            format!("{sysroot}/usr/src/uacpi/stdlib.c"),
            format!("{sysroot}/usr/src/uacpi/tables.c"),
            format!("{sysroot}/usr/src/uacpi/types.c"),
            format!("{sysroot}/usr/src/uacpi/uacpi.c"),
            format!("{sysroot}/usr/src/uacpi/utilities.c"),
        ])
        .includes([&include_path])
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
        .clang_args(["-I", &include_path, "-ffreestanding"])
        .header("./src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
