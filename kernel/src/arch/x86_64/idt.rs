//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::mem;

use bitflags::bitflags;

use crate::arch::x86_64::{cpu, registers::Registers};

bitflags! {
    struct Attribute: u8 {
        const NULL = 0;
        const TRAP_GATE = 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0;
        const INTERRUPT_GATE =  1 << 3 | 1 << 2 | 1 << 1 | 0 << 0;
        const KERNEL_PRIVILEGE = 0 << 6 | 0 << 5;
        const USER_PRIVILEGE = 1 << 6 | 1 << 5;
        const PRESENT = 1 << 7;
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
struct Entry {
    offset_low: u16,
    selector: u16,
    ist: u8,
    attributes: u8,
    offset_mid: u16,
    offset_high: u32,
    reserved: u32,
}

impl Entry {
    fn new(handler: *const u8, attribute: Attribute) -> Self {
        let address = handler as u64;
        Self {
            offset_low: (address & 0xffff) as u16,
            selector: 0x28,
            ist: 0,
            attributes: attribute.bits(),
            offset_mid: ((address >> 16) & 0xffff) as u16,
            offset_high: ((address >> 32) & 0xffffffff) as u32,
            reserved: 0,
        }
    }

    const fn default() -> Self {
        Self {
            offset_low: 0,
            selector: 0,
            ist: 0,
            attributes: 0,
            offset_mid: 0,
            offset_high: 0,
            reserved: 0,
        }
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
struct Descriptor {
    size: u16,
    address: u64,
}

impl Descriptor {
    fn new(size: u16, address: u64) -> Self {
        Self { size, address }
    }

    const fn default() -> Self {
        Self {
            size: 0,
            address: 0,
        }
    }
}

static mut ENTRIES: [Entry; 256] = [Entry::default(); 256];
static mut DESCRIPTOR: Descriptor = Descriptor::default();
static mut HANDLERS: [Option<fn(&mut Registers)>; 256] = [None; 256];

unsafe extern "C" {
    static interrupt_handlers: [*const u8; 256];

    fn load_idt(descriptor: *const Descriptor);
}

pub fn initialize() {
    unsafe {
        let entries = &raw mut ENTRIES;
        for i in 0..256 {
            (*entries)[i] = Entry::new(
                interrupt_handlers[i],
                Attribute::KERNEL_PRIVILEGE | Attribute::PRESENT | Attribute::INTERRUPT_GATE,
            );
        }

        DESCRIPTOR = Descriptor::new(
            (mem::size_of::<[Entry; 256]>() - 1) as u16,
            (&raw const ENTRIES as *const _) as u64,
        );
    }

    load();

    log::info!(target: "idt", "Initialized!");
}

pub fn load() {
    unsafe {
        load_idt(&raw const DESCRIPTOR);
    }
}

#[unsafe(no_mangle)]
extern "C" fn interrupt_raise(registers: *mut Registers) {
    let mut registers = unsafe { &mut *registers };

    if let Some(handler) = unsafe { HANDLERS[registers.isr as usize] } {
        handler(&mut registers);
    }

    // TODO: Implement EOI
    cpu::hcf();
}
