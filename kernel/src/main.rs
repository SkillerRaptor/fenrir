//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![no_std]
#![no_main]
#![feature(box_as_ptr)]
#![feature(cstr_display)]
#![feature(negative_impls)]

extern crate alloc;

mod acpi;
mod arch;
mod common;
mod drivers;
mod filesystem;
mod memory;
mod scheduler;
mod sync;
mod syscalls;

use core::{mem, panic::PanicInfo, ptr, sync::atomic::Ordering};

use elf::{
    ElfBytes,
    abi::{PF_W, PT_LOAD},
    endian::LittleEndian,
};

use crate::{
    acpi::{apic, hpet},
    arch::x86_64::{cpu, idt},
    common::{boot, logger, math, stacktrace},
    filesystem::ustar,
    memory::{
        PAGE_SIZE,
        pmm,
        vmm::{self, Attribute},
    },
    scheduler::smp,
};

#[unsafe(no_mangle)]
unsafe extern "C" fn kmain() -> ! {
    cpu::disable_interrupts();

    if !boot::is_base_revision_supported() {
        cpu::halt();
    }

    cpu::initialize_bsp();
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

    idt::initialize();

    pmm::initialize();
    vmm::initialize();

    stacktrace::initialize();

    acpi::initialize();
    hpet::initialize();
    apic::initialize();

    scheduler::initialize();
    smp::initialize();

    syscalls::initialize();

    let thread = scheduler::create_kernel_thread(kthread);
    scheduler::add_thread_to_current(&thread);

    scheduler::reschedule();
}

fn kthread() {
    log::info!("Fenrir successfully booted!");

    let initramfs = boot::get_modules()[1];
    let hello_world_bytes = ustar::lookup(initramfs.data(), "./hello_world").unwrap();
    let elf = ElfBytes::<LittleEndian>::minimal_parse(hello_world_bytes).unwrap();

    let mut highest_address = 0;
    let user_page_map = vmm::create_page_map();
    for program_header in elf
        .segments()
        .unwrap()
        .iter()
        .filter(|program_header| program_header.p_type == PT_LOAD)
    {
        let virtual_start = math::align_down(program_header.p_vaddr, PAGE_SIZE);
        let virtual_end =
            math::align_up(program_header.p_vaddr + program_header.p_memsz, PAGE_SIZE);
        let page_count = (virtual_end - virtual_start) / PAGE_SIZE;

        let physical_start = pmm::allocate(page_count, true) as u64;
        let mut attributes = Attribute::USER;
        if (program_header.p_flags & PF_W) == PF_W {
            attributes |= Attribute::WRITE;
        }

        for i in 0..page_count {
            let physical_address = physical_start + i * PAGE_SIZE;
            let virtual_address = virtual_start + i * PAGE_SIZE;
            vmm::map(user_page_map, physical_address, virtual_address, attributes);
        }

        let offset = program_header.p_offset as usize;
        let size = program_header.p_filesz as usize;

        let src = &hello_world_bytes[offset..offset + size];
        let dst = (physical_start + boot::get_hhdm_offset()) as *mut u8;

        let page_offset = (program_header.p_vaddr - virtual_start) as usize;
        unsafe {
            ptr::copy_nonoverlapping(src.as_ptr(), dst.add(page_offset), size);
        }

        if highest_address <= virtual_end {
            highest_address = virtual_end;
        }
    }

    log::info!("Creating user process...");

    let user_process = scheduler::create_process(user_page_map);
    user_process
        .heap_start
        .store(highest_address, Ordering::Release);
    user_process
        .heap_end
        .store(highest_address, Ordering::Release);

    let user_thread =
        scheduler::create_user_thread(&user_process, unsafe { mem::transmute(elf.ehdr.e_entry) });
    scheduler::add_thread_to_least_loaded(&user_thread);

    scheduler::reschedule();
}

#[panic_handler]
fn rust_panic(info: &PanicInfo) -> ! {
    log::error!("Panic at {}: {}", info.location().unwrap(), info.message());
    log::error!("");

    log::error!("Stacktrace:");
    stacktrace::print(50);

    cpu::hcf();
}
