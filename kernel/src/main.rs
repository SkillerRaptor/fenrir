//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![no_std]
#![no_main]
#![feature(cstr_display)]

extern crate alloc;

mod acpi;
mod arch;
mod common;
mod drivers;
mod memory;
mod sync;

use core::panic::PanicInfo;

use crate::{
    arch::x86_64::{cpu, gdt, idt, pic},
    common::{boot, logger, stacktrace},
    memory::{pmm, vmm},
};

#[unsafe(no_mangle)]
unsafe extern "C" fn kmain() -> ! {
    cpu::disable_interrupts();

    if !boot::is_base_revision_supported() {
        cpu::halt();
    }

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

    gdt::initialize();
    pic::disable();
    idt::initialize();

    pmm::initialize();
    vmm::initialize();

    stacktrace::initialize();

    acpi::initialize();

    cpu::enable_interrupts();

    log::info!("Hello, World!");

    cpu::hcf();
}

#[panic_handler]
fn rust_panic(info: &PanicInfo) -> ! {
    log::error!("Panic at {}: {}", info.location().unwrap(), info.message());
    log::error!("");

    log::error!("Stacktrace:");
    stacktrace::print(50);

    cpu::hcf();
}
