//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::arch::asm;

pub fn halt() {
    unsafe {
        asm!("hlt", options(nomem, nostack, preserves_flags));
    }
}

pub fn pause() {
    unsafe {
        asm!("pause", options(nomem, nostack, preserves_flags));
    }
}

pub fn enable_interrupts() {
    unsafe {
        asm!("sti", options(nomem, nostack, preserves_flags));
    }
}

pub fn disable_interrupts() {
    unsafe {
        asm!("cli", options(nomem, nostack, preserves_flags));
    }
}

pub fn hcf() -> ! {
    loop {
        disable_interrupts();
        halt();
    }
}

pub fn read_cr0() -> u64 {
    let value;

    unsafe {
        asm!("mov {}, cr0", out(reg) value, options(nomem, nostack, preserves_flags));
    }

    value
}

pub fn read_cr2() -> u64 {
    let value;

    unsafe {
        asm!("mov {}, cr2", out(reg) value, options(nomem, nostack, preserves_flags));
    }

    value
}

pub fn read_cr3() -> u64 {
    let value;

    unsafe {
        asm!("mov {}, cr3", out(reg) value, options(nomem, nostack, preserves_flags));
    }

    value
}

pub fn read_cr4() -> u64 {
    let value;

    unsafe {
        asm!("mov {}, cr4", out(reg) value, options(nomem, nostack, preserves_flags));
    }

    value
}

pub fn write_msr(msr: u32, value: u64) {
    let high = (value >> 32) & 0xffffffff;
    let low = (value >> 0) & 0xffffffff;

    unsafe {
        asm!("wrmsr",
            in("ecx") msr,
            in("eax") low,
            in("edx") high,
            options(nostack, preserves_flags)
        );
    }
}

pub fn read_msr(msr: u32) -> u64 {
    let mut high = 0;
    let mut low = 0;

    unsafe {
        asm!(
            "rdmsr",
            in("ecx") msr,
            out("eax") low,
            out("edx") high,
            options(nomem, nostack, preserves_flags)
        );
    }

    ((high as u64) << 32) | (low as u64)
}
