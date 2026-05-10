//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::sync::Arc;

use crate::{
    arch::x86_64::registers::Registers,
    memory::{PAGE_SIZE, pmm},
    scheduler::process::Process,
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

#[repr(C, align(16))]
pub struct FxState(pub [u8; 512]);

pub struct Thread {
    pub id: ThreadId,
    pub state: SpinLock<ThreadState>,
    pub registers: Registers,
    pub fx_state: FxState,
    pub fs_base: u64,
    pub stack: *mut u8,
    pub stack_size: usize,
    pub process: Arc<Process>,
}

unsafe impl Send for Thread {}
unsafe impl Sync for Thread {}

impl Drop for Thread {
    fn drop(&mut self) {
        if !self.stack.is_null() {
            pmm::free(self.stack, self.stack_size as u64 / PAGE_SIZE);
        }
    }
}
