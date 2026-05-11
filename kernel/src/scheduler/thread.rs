//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::sync::Arc;
use core::{
    mem,
    sync::atomic::{AtomicU32, Ordering},
};

use crate::{
    arch::x86_64::registers::Registers,
    common::boot,
    memory::{
        PAGE_SIZE,
        pmm,
        vmm::{self, Attribute},
    },
    scheduler::{self, process::Process},
    sync::spinlock::SpinLock,
};

#[repr(u8)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ThreadState {
    Idle,
    Busy,
    Dead,
}

#[repr(transparent)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct ThreadId(pub u32);

static NEXT_THREAD_ID: AtomicU32 = AtomicU32::new(0);

#[repr(C, align(16))]
pub struct FxState(pub [u8; 512]);

#[derive(Clone, Copy, PartialEq, Eq)]
enum ThreadLocation {
    Kernel,
    User,
}

pub struct Thread {
    pub id: ThreadId,
    pub state: SpinLock<ThreadState>,
    pub registers: Registers,
    pub fx_state: FxState,
    pub fs_base: u64,
    pub stack_base: *mut u8,
    pub stack_size: usize,
    pub process: Arc<Process>,
}

unsafe impl Send for Thread {}
unsafe impl Sync for Thread {}

impl Thread {
    pub fn new_kernel(entry: fn()) -> Arc<Self> {
        let stack_base = pmm::allocate(PAGE_SIZE, true) as u64;
        let virtual_stack = stack_base + boot::get_hhdm_offset();

        vmm::map(
            vmm::get_kernel_page_map(),
            stack_base,
            virtual_stack,
            Attribute::WRITE,
        );

        Self::new(
            scheduler::get_kernel_process(),
            ThreadLocation::Kernel,
            unsafe { mem::transmute(entry) },
            virtual_stack + PAGE_SIZE,
            stack_base,
            PAGE_SIZE as usize,
        )
    }

    pub fn new_user(
        process: &Arc<Process>,
        entry: u64,
        rsp: u64,
        stack_base: u64,
        stack_size: usize,
    ) -> Arc<Self> {
        Self::new(
            process,
            ThreadLocation::User,
            entry,
            rsp,
            stack_base,
            stack_size,
        )
    }

    fn new(
        process: &Arc<Process>,
        location: ThreadLocation,
        entry: u64,
        rsp: u64,
        stack_base: u64,
        stack_size: usize,
    ) -> Arc<Self> {
        let id = ThreadId(NEXT_THREAD_ID.fetch_add(1, Ordering::Relaxed));

        let (cs, ss) = if location == ThreadLocation::Kernel {
            (0x28, 0x30)
        } else {
            (0x40 | 0x03, 0x38 | 0x03)
        };

        let thread = Arc::new(Thread {
            id,
            state: SpinLock::new(ThreadState::Idle),
            registers: Registers {
                r15: 0,
                r14: 0,
                r13: 0,
                r12: 0,
                r11: 0,
                r10: 0,
                r9: 0,
                r8: 0,
                rsi: 0,
                rdi: 0,
                rbp: 0,
                rdx: 0,
                rcx: 0,
                rbx: 0,
                rax: 0,
                isr: 0,
                error: 0,
                rip: entry as u64,
                cs,
                flags: (1 << 9) | (1 << 1),
                rsp: rsp,
                ss,
            },
            fx_state: FxState([0; 512]),
            fs_base: 0,
            stack_base: stack_base as *mut u8,
            stack_size: stack_size,
            process: process.clone(),
        });

        scheduler::register_thread(&thread);
        process.threads.lock().push(thread.clone());

        thread
    }
}

impl Drop for Thread {
    fn drop(&mut self) {
        if !self.stack_base.is_null() {
            pmm::free(self.stack_base, self.stack_size as u64);
        }
    }
}
