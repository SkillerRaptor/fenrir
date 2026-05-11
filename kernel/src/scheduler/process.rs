//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::{sync::Arc, vec::Vec};
use core::sync::atomic::{AtomicU32, AtomicU64, Ordering};

use crate::{
    memory::vmm::PageMap,
    scheduler::{self, thread::Thread},
    sync::spinlock::SpinLock,
};

#[repr(transparent)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct ProcessId(pub u32);

static NEXT_PROCESS_ID: AtomicU32 = AtomicU32::new(0);

pub struct Process {
    pub id: ProcessId,
    pub page_map: PageMap,
    pub threads: SpinLock<Vec<Arc<Thread>>>,

    // NOTE: This is for user processes
    pub heap_start: AtomicU64,
    pub heap_end: AtomicU64,
}

unsafe impl Send for Process {}
unsafe impl Sync for Process {}

impl Process {
    pub fn new(page_map: PageMap) -> Arc<Self> {
        let id = ProcessId(NEXT_PROCESS_ID.fetch_add(1, Ordering::Relaxed));

        let process = Arc::new(Process {
            id,
            page_map,
            threads: SpinLock::new(Vec::new()),
            heap_start: AtomicU64::new(0),
            heap_end: AtomicU64::new(0),
        });

        scheduler::register_process(&process);

        process
    }
}
