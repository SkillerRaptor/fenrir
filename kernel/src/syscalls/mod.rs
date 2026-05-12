//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{ffi::CStr, slice, sync::atomic::Ordering};

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

    let syscall_id = registers.rax;
    let argument_1 = registers.rdi;
    let argument_2 = registers.rsi;
    let argument_3 = registers.rdx;
    let argument_4 = registers.rcx;
    let argument_5 = registers.r8;
    let argument_6 = registers.r9;

    match syscall_id {
        1 => {
            let size = argument_1;

            if false {
                log::debug!("Syscall: AnonAllocate(size: {:#x})", size);
            }

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
        2 => {
            let ptr = argument_1;
            let size = argument_2;

            log::debug!("Syscall: AnonFree(ptr: {:#018x}, size: {:#x})", ptr, size);

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        3 => {
            let fd = argument_1;

            log::debug!("Syscall: Close(fd: {})", fd);

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        4 => {
            let status = argument_1 as i64;

            log::debug!("Syscall: Exit(status: {})", status);

            let core = Core::current();
            core.enter_critical();
            let thread = core.current_thread.load(Ordering::Acquire);
            unsafe {
                *(*thread).state.lock() = ThreadState::Dead;
            }
            core.leave_critical();
            scheduler::reschedule();
        }
        5 => {
            let fd = argument_1;

            log::debug!("Syscall: Isatty(fd: {})", fd);

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        6 => {
            let path = argument_1;
            let mode = argument_2;

            log::debug!(
                "Syscall: Mkdir(path: {}, mode: {:#09b})",
                unsafe { CStr::from_ptr(path as *const i8).display() },
                mode
            );

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        7 => {
            let path = argument_1;
            let flags = argument_2;
            let mode = argument_3;

            log::debug!(
                "Syscall: Open(pathname: {}, flags: {}, mode: {:#09b})",
                unsafe { CStr::from_ptr(path as *const i8).display() },
                flags,
                mode
            );

            unsafe {
                (*registers_ptr).rax = u64::MAX;
            }
        }
        8 => {
            let fd = argument_1;
            let buffer = argument_2;
            let count = argument_2;

            log::debug!(
                "Syscall: Read(fd: {}, buffer: {:#018x}, count: {:#x})",
                fd,
                buffer,
                count
            );

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        9 => {
            let fd = argument_1;
            let offset = argument_2;
            let whence = argument_2;

            log::debug!(
                "Syscall: Seek(fd: {}, offset: {:#x}, whence: {})",
                fd,
                offset,
                whence
            );

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        10 => {
            let ptr = argument_1;

            if false {
                log::debug!("Syscall: TcbSet(ptr: {:#018x})", ptr);
            }

            cpu::set_fs_base(ptr);

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        11 => {
            let fd = argument_1;
            let buffer = argument_2;
            let count = argument_3;

            if fd != 1 && fd != 2 {
                log::debug!(
                    "Syscall: Write(fd: {}, buffer: {:#018x}, count: {:#x})",
                    fd,
                    buffer,
                    count
                );
            }

            let core = Core::current();
            let thread = core.current_thread.load(Ordering::Acquire);
            let page_map = unsafe { (*(*thread).process).page_map };

            let physical_address = vmm::virtual_to_physical(page_map, buffer);
            let kernel_ptr = (physical_address + boot::get_hhdm_offset()) as *const u8;

            let bytes = unsafe { slice::from_raw_parts(kernel_ptr, count as usize) };
            print!("{}", unsafe { str::from_utf8_unchecked(bytes) });

            unsafe {
                (*registers_ptr).rax = 0;
            }
        }
        _ => {
            log::warn!("Unhandled syscall: {}", syscall_id);

            unsafe {
                (*registers_ptr).rax = u64::MAX;
            }
        }
    }
}
