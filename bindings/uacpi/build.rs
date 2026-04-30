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
            "../../third_party/uacpi/source/default_handlers.c",
            "../../third_party/uacpi/source/event.c",
            "../../third_party/uacpi/source/interpreter.c",
            "../../third_party/uacpi/source/io.c",
            "../../third_party/uacpi/source/mutex.c",
            "../../third_party/uacpi/source/namespace.c",
            "../../third_party/uacpi/source/notify.c",
            "../../third_party/uacpi/source/opcodes.c",
            "../../third_party/uacpi/source/opregion.c",
            "../../third_party/uacpi/source/osi.c",
            "../../third_party/uacpi/source/registers.c",
            "../../third_party/uacpi/source/resources.c",
            "../../third_party/uacpi/source/shareable.c",
            "../../third_party/uacpi/source/stdlib.c",
            "../../third_party/uacpi/source/tables.c",
            "../../third_party/uacpi/source/types.c",
            "../../third_party/uacpi/source/uacpi.c",
            "../../third_party/uacpi/source/utilities.c",
        ])
        .includes(["../../third_party/uacpi/include/"])
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
        .clang_args(["-I", "../../third_party/uacpi/include/", "-ffreestanding"])
        .header("./src/wrapper.h")
        .generate()
        .expect("Unable to generate bindings!");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Unable to write bindings!");
}
