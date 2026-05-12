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

use alloc::{sync::Arc, vec::Vec};
use core::{panic::PanicInfo, ptr, sync::atomic::Ordering};

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
    scheduler::{process::Process, smp, thread::Thread},
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

    let thread = Thread::new_kernel(kthread);
    scheduler::add_thread_to_current(&thread);

    scheduler::reschedule();
}

fn load_program(bytes: &[u8], arguments: &[&str]) -> (Arc<Process>, Arc<Thread>) {
    let elf = ElfBytes::<LittleEndian>::minimal_parse(bytes).unwrap();

    let page_map = vmm::create_page_map();

    let mut virtual_stack = 0x00007fffffff0000;

    let mut highest_address = 0;
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
            vmm::map(page_map, physical_address, virtual_address, attributes);
        }

        let offset = program_header.p_offset as usize;
        let size = program_header.p_filesz as usize;

        let src = &bytes[offset..offset + size];
        let dst = (physical_start + boot::get_hhdm_offset()) as *mut u8;

        let page_offset = (program_header.p_vaddr - virtual_start) as usize;
        unsafe {
            ptr::copy_nonoverlapping(src.as_ptr(), dst.add(page_offset), size);
        }

        if highest_address <= virtual_end {
            highest_address = virtual_end;
        }
    }

    let stack_pages = 16;
    let stack_size = PAGE_SIZE * stack_pages;
    let stack = pmm::allocate(stack_pages, true) as u64;

    for i in 0..stack_pages {
        vmm::map(
            page_map,
            stack + i * PAGE_SIZE,
            virtual_stack - stack_size + i * PAGE_SIZE,
            Attribute::WRITE | Attribute::USER,
        );
    }

    let stack_hhdm = stack + boot::get_hhdm_offset();

    let mut physical_address = stack_hhdm + stack_size;
    let mut string_addresses: Vec<u64> = Vec::new();

    for argument in arguments.iter().rev() {
        let bytes = argument.as_bytes();
        physical_address -= 1;
        virtual_stack -= 1;
        unsafe { *(physical_address as *mut u8) = 0 };
        physical_address -= bytes.len() as u64;
        virtual_stack -= bytes.len() as u64;
        unsafe {
            ptr::copy_nonoverlapping(bytes.as_ptr(), physical_address as *mut u8, bytes.len())
        };
        string_addresses.push(virtual_stack);
    }
    string_addresses.reverse();

    virtual_stack = virtual_stack & !15;
    physical_address = physical_address & !15;

    let mut push = |value: u64| {
        virtual_stack -= 8;
        physical_address -= 8;
        unsafe { (physical_address as *mut u64).write(value) };
    };

    let word_count = 1 + string_addresses.len() + 1 + 1;
    if word_count & 1 != 0 {
        push(0);
    }

    push(0);
    push(0);

    push(elf.ehdr.e_entry as u64);
    push(9);

    push(elf.ehdr.e_phoff);
    push(3);

    push(elf.ehdr.e_phentsize as u64);
    push(4);

    push(elf.ehdr.e_phnum as u64);
    push(5);

    push(PAGE_SIZE);
    push(6);

    push(0);

    push(0);
    for addr in string_addresses.iter().rev() {
        push(*addr);
    }

    push(arguments.len() as u64);

    assert_eq!(virtual_stack % 16, 0);

    let user_process = Process::new(page_map);
    user_process
        .heap_start
        .store(highest_address, Ordering::Release);
    user_process
        .heap_end
        .store(highest_address, Ordering::Release);

    let user_thread = Thread::new_user(
        &user_process,
        elf.ehdr.e_entry,
        virtual_stack,
        stack,
        stack_size as usize,
    );
    (user_process, user_thread)
}

fn kthread() {
    log::info!("Fenrir successfully booted!");

    let initramfs = boot::get_modules()[1];
    let hello_world_bytes = ustar::lookup(initramfs.data(), "./doomgeneric").unwrap();
    let (_user_process, user_thread) =
        load_program(&hello_world_bytes, &["doomgeneric", "-iwad", "./DOOM1.WAD"]);
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
