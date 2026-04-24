//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![no_std]
#![no_main]

mod common;

use core::{arch::asm, panic::PanicInfo};

use crate::common::{boot, logger};

#[unsafe(no_mangle)]
unsafe extern "C" fn kmain() -> ! {
    assert!(boot::is_base_revision_supported());

    logger::initialize();

    println!("");
    println!("          _______ _    _ _____ _______ _______ _     _ _______ __   _");
    println!("   |      |______  \\  /    |   |_____|    |    |_____| |_____| | \\  |");
    println!("   |_____ |______   \\/   __|__ |     |    |    |     | |     | |  \\_|");
    println!("");
    println!(
        "   Bootloader: {} {}",
        boot::get_bootloader_name(),
        boot::get_bootloader_version(),
    );
    println!("   Firmware: {}", boot::get_firmware_type());
    println!("");

    log::info!("Hello, World!");

    loop {
        unsafe {
            asm!("hlt");
        }
    }
}

#[panic_handler]
fn rust_panic(_info: &PanicInfo) -> ! {
    loop {
        unsafe {
            asm!("cli");
            asm!("hlt");
        }
    }
}
