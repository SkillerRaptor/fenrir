//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::arch::asm;

pub fn halt() {
    unsafe {
        asm!("hlt");
    }
}

pub fn pause() {
    unsafe {
        asm!("pause");
    }
}

pub fn enable_interrupts() {
    unsafe {
        asm!("sti");
    }
}

pub fn disable_interrupts() {
    unsafe {
        asm!("cli");
    }
}

pub fn hcf() -> ! {
    loop {
        disable_interrupts();
        halt();
    }
}
