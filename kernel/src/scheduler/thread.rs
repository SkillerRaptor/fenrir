//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::sync::Arc;

use crate::{arch::x86_64::registers::Registers, scheduler::process::Process};

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ThreadState {
    Idle,
    Busy,
    Dead,
}

#[repr(transparent)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct ThreadId(pub u32);

pub struct Thread {
    pub id: ThreadId,
    pub state: ThreadState,

    pub registers: Registers,
    pub stack: *const u8,
    pub stack_size: usize,

    pub process: Arc<Process>,
}

unsafe impl Send for Thread {}
unsafe impl Sync for Thread {}
