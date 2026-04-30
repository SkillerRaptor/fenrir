//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use std::{
    error::Error,
    ffi::OsString,
    fs::{self, DirEntry},
    io,
    path::Path,
    result::Result,
};

fn visit_directory(directory: &Path, callback: &mut dyn FnMut(&DirEntry)) -> io::Result<()> {
    if !directory.is_dir() {
        return Ok(());
    }

    for entry in fs::read_dir(directory)? {
        let entry = entry?;
        let path = entry.path();

        if path.is_dir() {
            visit_directory(&path, callback)?;
            continue;
        }

        callback(&entry);
    }

    Ok(())
}

fn main() -> Result<(), Box<dyn Error>> {
    visit_directory(Path::new("src"), &mut |entry: &DirEntry| {
        let path = entry.path();

        let object_os = path.file_name().expect("Failed to get file name");
        let object_file = object_os.to_str().expect("Invalid UTF-8 for file name");

        match path.extension() {
            Some(extension) if extension.eq(&OsString::from("asm")) => {
                let mut build = nasm_rs::Build::new();

                build
                    .file(&path)
                    .flag("-felf64")
                    .target("x86_64-unknown-none")
                    .compile(object_file)
                    .expect("Failed to compile assembly");

                println!("cargo:rustc-link-lib=static={}", object_file);
                println!("cargo:rerun-if-changed={}", object_file);
            }

            _ => (),
        }
    })?;

    println!("cargo:rustc-link-arg=-T./linker_scripts/x86_64.ld");
    println!("cargo:rerun-if-changed=./linker_scripts/x86_64.ld");

    Ok(())
}
