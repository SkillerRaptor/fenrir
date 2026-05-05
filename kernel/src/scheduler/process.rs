//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::{sync::Arc, vec::Vec};

use crate::{memory::vmm::PageMap, scheduler::thread::Thread, sync::spinlock::SpinLock};

#[repr(transparent)]
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct ProcessId(pub u32);

pub struct Process {
    pub id: ProcessId,
    pub page_map: PageMap,

    pub threads: SpinLock<Vec<Arc<Thread>>>,
}

unsafe impl Send for Process {}
unsafe impl Sync for Process {}
