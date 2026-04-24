//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::mem;

use bitflags::bitflags;

bitflags! {
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

    struct FlagAttribute: u8 {
        const NULL = 0;
        const LONG_MODE = 1 << 1;
        const SIZE_32 = 1 << 2;
        const PAGE_GRANULARITY = 1 << 3;
    }
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
struct Entry {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    limit_high_flags: u8,
    base_high: u8,
}

impl Entry {
    const fn new(base: u32, limit: u32, access: AccessAttribute, flags: FlagAttribute) -> Self {
        Self {
            limit_low: limit as u16,
            base_low: base as u16,
            base_middle: (base >> 16) as u8,
            access: access.bits(),
            limit_high_flags: (flags.bits() << 4) & 0xf0 | ((limit >> 16) as u8) & 0x0f,
            base_high: (base >> 24) as u8,
        }
    }

    const fn default() -> Self {
        Self {
            limit_low: 0,
            base_low: 0,
            base_middle: 0,
            access: 0,
            limit_high_flags: 0,
            base_high: 0,
        }
    }
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
struct Descriptor {
    size: u16,
    address: u64,
}

impl Descriptor {
    const fn new(size: u16, address: u64) -> Self {
        Self { size, address }
    }

    const fn default() -> Self {
        Self {
            size: 0,
            address: 0,
        }
    }
}

static mut ENTRIES: [Entry; 7] = [Entry::default(); 7];
static mut DESCRIPTOR: Descriptor = Descriptor::default();

unsafe extern "C" {
    fn load_gdt(descriptor: *const Descriptor);

    fn reload_segments();
}

pub fn initialize() {
    unsafe {
        ENTRIES = [
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
        ];

        DESCRIPTOR = Descriptor::new(
            (mem::size_of::<[Entry; 7]>() - 1) as u16,
            (&raw const ENTRIES as *const _) as u64,
        );
    }

    load();

    log::info!(target: "gdt", "Initialized!");
}

pub fn load() {
    unsafe {
        load_gdt(&raw const DESCRIPTOR);
        reload_segments();
    }
}
