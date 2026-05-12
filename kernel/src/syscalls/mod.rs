//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{
    arch::asm,
    ffi::CStr,
    ptr,
    slice,
    sync::atomic::{AtomicU64, Ordering},
};

use crate::{
    acpi::hpet,
    arch::x86_64::cpu::{self, Core},
    common::{boot, logger, math},
    filesystem::ustar,
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

static OFFSET: AtomicU64 = AtomicU64::new(0);

#[unsafe(no_mangle)]
fn syscall_handler(registers_ptr: *mut Registers) {
    let registers = unsafe { &*registers_ptr };

    let syscall_id = registers.rax;
    let argument_1 = registers.rdi;
    let argument_2 = registers.rsi;
    let argument_3 = registers.rdx;
    let argument_4 = registers.r10;
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
            let physical_base = pmm::allocate(page_count, true) as u64;

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
            if status == 0 {
                log::info!("The thread has exited with code {}", status);
            } else {
                log::error!("The thread has exited with code {}", status);
            }

            if false {
                log::debug!("Syscall: Exit(status: {})", status);
            }

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

            if false {
                log::debug!("Syscall: Isatty(fd: {})", fd);
            }

            const ENOTTY: u64 = 25;

            unsafe {
                (*registers_ptr).rax = ENOTTY;
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

            let path = unsafe { CStr::from_ptr(path as *const i8) }
                .to_str()
                .unwrap();

            log::debug!(
                "Syscall: Open(pathname: {}, flags: {}, mode: {:#09b})",
                path,
                flags,
                mode
            );

            if path == "./DOOM1.WAD" {
                // NOTE: Hardcode FD to 10
                unsafe {
                    (*registers_ptr).rax = 10;
                }
            } else {
                unsafe {
                    (*registers_ptr).rax = u64::MAX;
                }
            }
        }
        8 => {
            let fd = argument_1;
            let buffer = argument_2;
            let count = argument_3;

            if fd == 10 {
                let core = Core::current();
                let thread = core.current_thread.load(Ordering::Acquire);
                let page_map = unsafe { (*(*thread).process).page_map };

                let physical_address = vmm::virtual_to_physical(page_map, buffer);
                let buffer = (physical_address + boot::get_hhdm_offset()) as *mut u8;

                let initramfs = boot::get_modules()[1];
                let bytes = ustar::lookup(initramfs.data(), "./DOOM1.WAD").unwrap();

                unsafe {
                    ptr::copy_nonoverlapping(
                        bytes
                            .as_ptr()
                            .add(OFFSET.fetch_add(count, Ordering::Acquire) as usize),
                        buffer,
                        count as usize,
                    );
                }

                unsafe {
                    (*registers_ptr).rax = count;
                }
            } else {
                log::debug!(
                    "Syscall: Read(fd: {}, buffer: {:#018x}, count: {:#x})",
                    fd,
                    buffer,
                    count
                );

                unsafe {
                    (*registers_ptr).rax = u64::MAX;
                }
            }
        }
        9 => {
            let fd = argument_1;
            let offset = argument_2;
            let whence = argument_3 as u8;

            const SEEK_SET: u8 = 0;
            const SEEK_CUR: u8 = 1;
            const SEEK_END: u8 = 2;

            let offset = if fd == 10 {
                match whence {
                    SEEK_SET => {
                        OFFSET.store(offset, Ordering::Release);
                        offset
                    }
                    SEEK_CUR => {
                        OFFSET.fetch_add(offset, Ordering::Release);
                        OFFSET.load(Ordering::Acquire)
                    }
                    SEEK_END => {
                        let initramfs = boot::get_modules()[1];
                        let bytes = ustar::lookup(initramfs.data(), "./DOOM1.WAD").unwrap();
                        bytes.len() as u64 + offset
                    }
                    _ => unreachable!(),
                }
            } else {
                log::debug!(
                    "Syscall: Seek(fd: {}, offset: {:#x}, whence: {})",
                    fd,
                    offset,
                    whence
                );

                u64::MAX
            };

            unsafe {
                (*registers_ptr).rax = offset;
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
        60 => {
            let core = Core::current();
            core.enter_critical();
            let thread = core.current_thread.load(Ordering::Acquire);
            let page_map = unsafe { (*(*thread).process).page_map };

            let framebuffer = boot::get_framebuffers()[0];
            let physical_address = framebuffer.address() as u64 - boot::get_hhdm_offset();
            let byte_size = framebuffer.pitch * framebuffer.height;

            let aligned_physical_address = math::align_down(physical_address, PAGE_SIZE);
            let physical_offset = physical_address - aligned_physical_address;

            let total_bytes = byte_size + physical_offset;
            let total_pages = math::div_round_up(total_bytes, PAGE_SIZE);

            for i in 0..total_pages {
                vmm::map(
                    page_map,
                    aligned_physical_address + i * PAGE_SIZE,
                    0x0000000010000000 + i * PAGE_SIZE,
                    Attribute::USER | Attribute::WRITE,
                );
            }
            core.leave_critical();

            unsafe {
                (*registers_ptr).rax = 0x0000000010000000;
            }
        }
        61 => unsafe {
            (*registers_ptr).rax = boot::get_framebuffers()[0].pitch;
        },
        62 => {
            let ms = argument_1;
            // hpet::sleep(ms);
        }
        63 => {
            fn get_tsc() -> u64 {
                let mut low = 0u32;
                let mut high = 0u32;
                unsafe {
                    asm!(
                        "rdtsc",
                        out("eax") low,
                        out("edx") high,
                        options(nomem, nostack));
                }

                return ((high as u64) << 32) | (low as u64);
            }

            // NOTE: THIS IS VERY HACKY
            let elapsed = get_tsc() - logger::TSC_BOOT.get();
            let milliseconds = (elapsed * 1000) / boot::get_tsc_frequency();

            unsafe {
                (*registers_ptr).rax = milliseconds;
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
