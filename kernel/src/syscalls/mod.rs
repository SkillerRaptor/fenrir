//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use crate::arch::x86_64::cpu;

const STAR_MSR: u32 = 0xc0000081;
const LSTAR_MSR: u32 = 0xc0000082;
const SFMASK_MSR: u32 = 0xc0000084;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Registers {
    pub r15: u64,
    pub r14: u64,
    pub r13: u64,
    pub r12: u64,
    pub r11: u64,
    pub r10: u64,
    pub r9: u64,
    pub r8: u64,

    pub rsi: u64,
    pub rdi: u64,
    pub rbp: u64,
    pub rdx: u64,
    pub rcx: u64,
    pub rbx: u64,
    pub rax: u64,
}

unsafe extern "C" {
    fn syscall_entry();
}

pub fn initialize() {
    cpu::write_msr(0xc0000080, cpu::read_msr(0xc0000080) | (1 << 0));

    cpu::write_msr(STAR_MSR, ((0x28 as u64) << 32) | ((0x30 as u64) << 48));
    cpu::write_msr(LSTAR_MSR, syscall_entry as *const () as u64);
    cpu::write_msr(SFMASK_MSR, (1 << 10) | (1 << 9));
}

#[unsafe(no_mangle)]
fn syscall_handler(registers: *mut Registers) {
    log::info!("Received syscall: {}", unsafe { (*registers).rax });
}
