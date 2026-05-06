//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::mem;

use bitflags::bitflags;

use crate::arch::x86_64::cpu::Core;

bitflags! {
    #[derive(Clone, Copy)]
    struct AccessAttribute: u8 {
        const NULL = 0;
        const ACCESS = 1 << 0;
        const READ_WRITE = 1 << 1;
        const DIRECTION = 1 << 2;
        const EXECUTABLE = 1 << 3;
        const CODE_DATA = 1 << 4;
        const KERNEL_PRIVILEGE = 0 << 6 | 0 << 5;
        const USER_PRIVILEGE = 1 << 6 | 1 << 5;
        const PRESENT = 1 << 7;
    }

    #[derive(Clone, Copy)]
    struct FlagAttribute: u8 {
        const NULL = 0;
        const LONG_MODE = 1 << 1;
        const SIZE_32 = 1 << 2;
        const PAGE_GRANULARITY = 1 << 3;
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
struct Tss {
    reserved_0: u32,
    rsp_0: u64,
    rsp_1: u64,
    rsp_2: u64,
    reserved_1: u64,
    ist_1: u64,
    ist_2: u64,
    ist_3: u64,
    ist_4: u64,
    ist_5: u64,
    ist_6: u64,
    ist_7: u64,
    reserved_2: u64,
    reserved_3: u16,
    io_offset: u16,
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
struct Entry {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    limit_high_flags: u8,
    base_high: u8,
}

impl Entry {
    fn new(base: u32, limit: u32, access: AccessAttribute, flags: FlagAttribute) -> Self {
        Self {
            limit_low: limit as u16,
            base_low: base as u16,
            base_middle: (base >> 16) as u8,
            access: access.bits(),
            limit_high_flags: (flags.bits() << 4) & 0xf0 | ((limit >> 16) as u8) & 0x0f,
            base_high: (base >> 24) as u8,
        }
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
struct TssEntry {
    limit_low: u16,
    base_low: u16,
    base_middle_1: u8,
    access: u8,
    limit_high_flags: u8,
    base_middle_2: u8,
    base_high: u32,
    reserved: u32,
}

impl TssEntry {
    fn new(address: u64) -> Self {
        const LIMIT: usize = mem::size_of::<Tss>();
        Self {
            limit_low: LIMIT as u16,
            base_low: address as u16,
            base_middle_1: (address >> 16) as u8,
            access: (AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::EXECUTABLE
                | AccessAttribute::ACCESS)
                .bits(),
            limit_high_flags: (FlagAttribute::PAGE_GRANULARITY.bits() << 4) & 0xf0
                | ((LIMIT >> 16) as u8) & 0x0f,
            base_middle_2: (address >> 24) as u8,
            base_high: (address >> 32) as u32,
            reserved: 0,
        }
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
struct Table {
    entries: [Entry; 9],
    tss_entry: TssEntry,
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
struct Descriptor {
    size: u16,
    address: u64,
}

impl Descriptor {
    fn new(size: u16, address: u64) -> Self {
        Self { size, address }
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
pub struct Gdt {
    tss: Tss,
    table: Table,
    descriptor: Descriptor,
}

unsafe extern "C" {
    fn load_gdt(descriptor: *const Descriptor);
    fn load_tss();

    fn reload_segments();
}

pub fn load(core: &Core) {
    let gdt = unsafe { &mut *core.gdt.get() };

    gdt.tss = Tss {
        reserved_0: 0,
        rsp_0: unsafe { *core.kernel_rsp.get() },
        rsp_1: 0,
        rsp_2: 0,
        reserved_1: 0,
        ist_1: 0,
        ist_2: 0,
        ist_3: 0,
        ist_4: 0,
        ist_5: 0,
        ist_6: 0,
        ist_7: 0,
        reserved_2: 0,
        reserved_3: 0,
        io_offset: 0,
    };

    gdt.table.entries = [
        Entry::new(
            0x00000000,
            0x00000000,
            AccessAttribute::NULL,
            FlagAttribute::NULL,
        ),
        Entry::new(
            0x00000000,
            0x0000ffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::EXECUTABLE
                | AccessAttribute::READ_WRITE,
            FlagAttribute::NULL,
        ),
        Entry::new(
            0x00000000,
            0x0000ffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::NULL,
        ),
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::EXECUTABLE
                | AccessAttribute::READ_WRITE,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::SIZE_32,
        ),
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::SIZE_32,
        ),
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::EXECUTABLE
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::LONG_MODE,
        ),
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::KERNEL_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::LONG_MODE,
        ),
        // NOTE: User segments are in reversed order
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::USER_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::LONG_MODE,
        ),
        Entry::new(
            0x00000000,
            0xffffffff,
            AccessAttribute::PRESENT
                | AccessAttribute::USER_PRIVILEGE
                | AccessAttribute::CODE_DATA
                | AccessAttribute::EXECUTABLE
                | AccessAttribute::READ_WRITE
                | AccessAttribute::ACCESS,
            FlagAttribute::PAGE_GRANULARITY | FlagAttribute::LONG_MODE,
        ),
    ];

    gdt.table.tss_entry = TssEntry::new((&gdt.tss as *const _) as u64);

    gdt.descriptor = Descriptor::new(
        (mem::size_of::<Table>() - 1) as u16,
        (&raw const gdt.table as *const _) as u64,
    );

    unsafe {
        load_gdt(&gdt.descriptor);
        reload_segments();
        load_tss();
    }
}
