//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{slice, sync::atomic::Ordering};

use crate::{
    arch::x86_64::cpu::{self, Core},
    common::{boot, math},
    memory::{
        PAGE_SIZE,
        pmm,
        vmm::{self, Attribute},
    },
    print,
    scheduler::{self, thread::ThreadState},
};

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
fn syscall_handler(registers_ptr: *mut Registers) {
    let registers = unsafe { &*registers_ptr };

    match registers.rax {
        1 => {
            let core = Core::current();
            core.enter_critical();
            let thread = core.current_thread.load(Ordering::Acquire);
            unsafe {
                *(*thread).state.lock() = ThreadState::Dead;
            }
            core.leave_critical();
            scheduler::reschedule();
        }
        2 => {
            let ptr = registers.rdi as u64;
            let length = registers.rsi as usize;

            let core = Core::current();
            let thread = core.current_thread.load(Ordering::Acquire);
            let page_map = unsafe { (*(*thread).process).page_map };

            let physical_address = vmm::virtual_to_physical(page_map, ptr);
            let kernel_ptr = (physical_address + boot::get_hhdm_offset()) as *const u8;

            let bytes = unsafe { slice::from_raw_parts(kernel_ptr, length) };
            print!("{}", unsafe { str::from_utf8_unchecked(bytes) });
        }
        3 => {
            let size = registers.rdi;

            let core = Core::current();
            let thread = core.current_thread.load(Ordering::Acquire);
            let process = unsafe { &(*thread).process };

            let old_heap_end = process.heap_end.load(Ordering::Acquire);
            let new_heap_end = math::align_up(old_heap_end + size, PAGE_SIZE);

            let page_count = (new_heap_end - old_heap_end) / PAGE_SIZE;
            let physical_base = pmm::allocate(page_count, false) as u64;

            for i in 0..page_count {
                let physical_address = physical_base + i * PAGE_SIZE;
                let virtual_address = old_heap_end + i * PAGE_SIZE;
                vmm::map(
                    process.page_map,
                    physical_address,
                    virtual_address,
                    Attribute::WRITE | Attribute::USER,
                );
            }

            process.heap_end.store(new_heap_end, Ordering::Release);

            unsafe {
                (*registers_ptr).rax = old_heap_end;
            }
        }
        _ => log::debug!("Received syscall: {}", registers.rax),
    }
}
