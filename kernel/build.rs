//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

fn main() {
    println!("cargo:rustc-link-arg=-T./linker_scripts/x86_64.ld");
    println!("cargo:rerun-if-changed=./linker_scripts/x86_64.ld");
}
