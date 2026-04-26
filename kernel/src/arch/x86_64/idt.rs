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
static mut HANDLERS: [Option<fn(&Registers)>; 256] = [None; 256];

static EXCEPTIONS: [&'static str; 31] = [
    "Divide-by-zero Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "<invalid>",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment-Fault",
    "General-Protection-Fault",
    "Page Fault",
    "<invalid>",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "Security Exception",
];

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

        HANDLERS[0] = Some(handle_exception);
        HANDLERS[1] = Some(handle_exception);
        HANDLERS[2] = Some(handle_exception);
        HANDLERS[3] = Some(handle_exception);
        HANDLERS[4] = Some(handle_exception);
        HANDLERS[5] = Some(handle_exception);
        HANDLERS[6] = Some(handle_exception);
        HANDLERS[7] = Some(handle_exception);
        HANDLERS[8] = Some(handle_exception);

        HANDLERS[10] = Some(handle_exception);
        HANDLERS[11] = Some(handle_exception);
        HANDLERS[12] = Some(handle_exception);
        HANDLERS[13] = Some(handle_exception);
        HANDLERS[14] = Some(handle_exception);

        HANDLERS[16] = Some(handle_exception);
        HANDLERS[17] = Some(handle_exception);
        HANDLERS[18] = Some(handle_exception);
        HANDLERS[19] = Some(handle_exception);
        HANDLERS[20] = Some(handle_exception);

        HANDLERS[30] = Some(handle_exception);

        DESCRIPTOR = Descriptor::new(
            (mem::size_of::<[Entry; 256]>() - 1) as u16,
            (&raw const ENTRIES as *const _) as u64,
        );
    }

    load();

    log::info!("IDT: Initialized");
}

pub fn load() {
    unsafe {
        load_idt(&raw const DESCRIPTOR);
    }
}

fn handle_exception(registers: &Registers) {
    log::error!("");
    log::error!("{} occured!", EXCEPTIONS[registers.isr as usize]);

    match registers.isr {
        0x0e => {
            log::error!("  at {:#016x}", cpu::read_cr2());

            let error = registers.error;
            log::error!(
                "  because {}, {}, {}{}{}",
                if error & (1 << 0) != 0 {
                    "protection violation"
                } else {
                    "non-present page"
                },
                if error & (1 << 1) != 0 {
                    "write access"
                } else {
                    "read access"
                },
                if error & (1 << 2) != 0 {
                    "user-mode"
                } else {
                    "kernel-mode"
                },
                if error & (1 << 3) != 0 {
                    ", reserved bit set in PTE"
                } else {
                    ""
                },
                if error & (1 << 4) != 0 {
                    ", instruction fetch (NX fault)"
                } else {
                    ""
                }
            );
        }

        0x0a | 0x0b | 0x0c | 0x0d => 'block: {
            if registers.error == 0 {
                break 'block;
            }

            let table = (registers.error >> 1) & 0b11;
            let index = (registers.error >> 3) & 0x1fff;

            let table_name = match table {
                0 => "GDT",
                1 | 3 => "IDT",
                2 => "LDT",
                _ => unreachable!(),
            };

            log::error!("  in {} at at {}", table_name, index);
        }
        _ => {}
    };
    log::error!("");

    log::error!("Registers:");
    log::error!(
        "  rax={:#018x} rbx={:#018x} rcx={:#018x} rdx={:#018x}",
        registers.rax,
        registers.rbx,
        registers.rcx,
        registers.rdx
    );
    log::error!(
        "  rsi={:#018x} rdi={:#018x} rbp={:#018x} rsp={:#018x}",
        registers.rsi,
        registers.rdi,
        registers.rbp,
        registers.rsp
    );
    log::error!(
        "   r8={:#018x}  r9={:#018x} r10={:#018x} r11={:#018x}",
        registers.r8,
        registers.r9,
        registers.r10,
        registers.r11
    );
    log::error!(
        "  r12={:#018x} r13={:#018x} r14={:#018x} r15={:#018x}",
        registers.r12,
        registers.r13,
        registers.r14,
        registers.r15
    );
    log::error!(
        "  rip={:#018x} rfl={:#018x}",
        registers.rip,
        registers.flags
    );
    log::error!("");

    log::error!("Segments:");
    log::error!("  cs={:#04x}", registers.cs);
    log::error!("  ss={:#04x}", registers.ss);
    log::error!("");

    log::error!("Control Registers:");
    log::error!(
        "  cr0={:#018x}  cr2={:#018x}",
        cpu::read_cr0(),
        cpu::read_cr2()
    );
    log::error!(
        "  cr3={:#018x}  cr4={:#018x}",
        cpu::read_cr3(),
        cpu::read_cr4()
    );
    log::error!("");

    log::error!("Halting System...");
    log::error!("");

    cpu::hcf();
}

#[unsafe(no_mangle)]
extern "C" fn interrupt_raise(registers: *mut Registers) {
    let registers = unsafe { &*registers };

    if let Some(handler) = unsafe { HANDLERS[registers.isr as usize] } {
        handler(&registers);
    }

    // TODO: Implement EOI
    cpu::hcf();
}
