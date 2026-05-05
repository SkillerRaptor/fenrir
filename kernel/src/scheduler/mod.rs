//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub mod process;
pub mod smp;
pub mod thread;

use alloc::{collections::BTreeMap, sync::Arc, vec::Vec};
use core::{
    mem,
    sync::atomic::{AtomicU32, Ordering},
};

use crate::{
    acpi::apic::{self, TIMER_ISR},
    arch::x86_64::{
        cpu::{self, Core},
        idt,
        registers::Registers,
    },
    common::{boot, once::Once},
    memory::{
        PAGE_SIZE,
        pmm,
        vmm::{self, Attribute, PageMap},
    },
    scheduler::{
        process::{Process, ProcessId},
        thread::{Thread, ThreadId, ThreadState},
    },
    sync::spinlock::SpinLock,
};

static NEXT_PROCESS_ID: AtomicU32 = AtomicU32::new(0);
static NEXT_THREAD_ID: AtomicU32 = AtomicU32::new(0);

static PROCESSES: SpinLock<BTreeMap<ProcessId, Arc<Process>>> = SpinLock::new(BTreeMap::new());
static THREADS: SpinLock<BTreeMap<ThreadId, Arc<Thread>>> = SpinLock::new(BTreeMap::new());

static KERNEL_PROCESS: Once<Arc<Process>> = Once::new();

pub fn initialize() {
    idt::set_handler(TIMER_ISR as u8, schedule);

    unsafe {
        KERNEL_PROCESS.initialize(create_process(vmm::get_kernel_page_map()));
    }

    log::info!("Scheduler: Initialized");
}

pub fn reschedule() -> ! {
    loop {
        cpu::enable_interrupts();
        cpu::halt();
    }
}

pub fn create_process(page_map: PageMap) -> Arc<Process> {
    let id = ProcessId(NEXT_PROCESS_ID.fetch_add(1, Ordering::Relaxed));

    let process = Arc::new(Process {
        id,
        page_map,
        threads: SpinLock::new(Vec::new()),
    });

    PROCESSES.lock().insert(id, process.clone());

    process
}

pub fn create_kernel_thread(entry: fn()) -> Arc<Thread> {
    create_thread(KERNEL_PROCESS.get(), 0x28, entry)
}

fn create_thread(process: &Arc<Process>, cs: u64, entry: fn()) -> Arc<Thread> {
    let page_map = process.page_map;

    let id = ThreadId(NEXT_THREAD_ID.fetch_add(1, Ordering::Relaxed));

    let stack = pmm::allocate(1, true) as u64;
    vmm::map(
        page_map,
        stack,
        stack + boot::get_hhdm_offset(),
        Attribute::WRITE,
    );

    let thread = Arc::new(Thread {
        id,
        state: ThreadState::Idle,
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
            rsp: stack + boot::get_hhdm_offset() + PAGE_SIZE,
            ss: cs + 0x08,
        },
        stack: stack as *const u8,
        stack_size: PAGE_SIZE as usize,
        process: process.clone(),
    });

    THREADS.lock().insert(id, thread.clone());
    process.threads.lock().push(thread.clone());

    thread
}

pub fn schedule_thread(thread: &Arc<Thread>) {
    let core = Core::current();
    core.thread_queue.lock().push_back(thread.clone());
}

unsafe extern "C" {
    fn switch_process(registers: *const Registers) -> !;
}

fn schedule(registers: &Registers) {
    apic::send_eoi();

    let core = Core::current();

    let current_thread = core.current_thread.load(Ordering::Acquire);
    if !current_thread.is_null() && unsafe { (*current_thread).state } == ThreadState::Dead {
        // TODO: Add thread reaping
    }

    if core.thread_queue.lock().is_empty() {
        if !current_thread.is_null() {
            return;
        }

        let idle_thread = core.idle_thread.load(Ordering::Acquire);
        let idle_registers = unsafe { &(*idle_thread).registers };

        unsafe { switch_process(&raw const *idle_registers) };
    }

    let next_thread = core.thread_queue.lock().pop_front().unwrap();
    let next_thread = Arc::into_raw(next_thread.clone()) as *mut Thread;

    core.current_thread.store(next_thread, Ordering::Release);

    if !current_thread.is_null() {
        unsafe {
            (*current_thread).state = ThreadState::Idle;
            (*current_thread).registers = *registers;
        }
    }

    unsafe {
        if current_thread.is_null()
            || (&(*current_thread)).process.id != (&(*next_thread)).process.id
        {
            vmm::switch_to_page_map((&(*next_thread)).process.page_map);
        }

        (*next_thread).state = ThreadState::Busy;
    }

    if !current_thread.is_null() {
        core.thread_queue.lock().push_back({
            let current_thread = unsafe { Arc::from_raw(current_thread) };
            let clone = current_thread.clone();
            mem::forget(current_thread);
            clone
        });
    }

    unsafe { switch_process(&raw const (*next_thread).registers) };
}
