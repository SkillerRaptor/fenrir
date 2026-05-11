//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub mod process;
pub mod smp;
pub mod thread;

use alloc::{collections::BTreeMap, sync::Arc};
use core::{mem, ptr, sync::atomic::Ordering};

use crate::{
    acpi::apic::{self, TIMER_ISR},
    arch::x86_64::{
        cpu::{self, Core},
        idt,
        registers::Registers,
    },
    common::once::Once,
    memory::vmm,
    scheduler::{
        process::{Process, ProcessId},
        thread::{Thread, ThreadId, ThreadState},
    },
    sync::spinlock::SpinLock,
};

static PROCESSES: SpinLock<BTreeMap<ProcessId, Arc<Process>>> = SpinLock::new(BTreeMap::new());
static THREADS: SpinLock<BTreeMap<ThreadId, Arc<Thread>>> = SpinLock::new(BTreeMap::new());

static KERNEL_PROCESS: Once<Arc<Process>> = Once::new();

pub fn initialize() {
    idt::set_handler(TIMER_ISR as u8, schedule);

    unsafe {
        KERNEL_PROCESS.initialize(Process::new(vmm::get_kernel_page_map()));
    }

    log::info!("Scheduler: Initialized");
}

pub fn register_process(process: &Arc<Process>) {
    PROCESSES.lock().insert(process.id, process.clone());
}

pub fn register_thread(thread: &Arc<Thread>) {
    THREADS.lock().insert(thread.id, thread.clone());
}

pub fn get_kernel_process() -> &'static Arc<Process> {
    KERNEL_PROCESS.get()
}

pub fn add_thread_to_current(thread: &Arc<Thread>) {
    let core = Core::current();
    core.thread_queue.lock().push_back(thread.clone());
}

pub fn add_thread_to_least_loaded(thread: &Arc<Thread>) {
    let core = Core::all()
        .min_by_key(|core| {
            let has_running_thread = !core.current_thread.load(Ordering::Acquire).is_null();
            core.thread_queue.lock().len() + if has_running_thread { 1 } else { 0 }
        })
        .unwrap();
    core.thread_queue.lock().push_back(thread.clone());
}

pub fn reschedule() -> ! {
    loop {
        cpu::enable_interrupts();
        cpu::halt();
    }
}

unsafe extern "C" {
    fn switch_process(registers: *const Registers) -> !;
}

fn schedule(registers: &Registers) {
    let core = Core::current();

    let current_thread = core.current_thread.load(Ordering::Acquire);
    let next_thread = core.thread_queue.lock().pop_front();

    if !current_thread.is_null() {
        unsafe {
            (*current_thread).registers = *registers;
            cpu::fxsave(&mut (*current_thread).fx_state);
            (*current_thread).fs_base = cpu::read_msr(0xc0000100);
        }

        let current_thread = unsafe { Arc::from_raw(current_thread) };
        if *current_thread.state.lock() == ThreadState::Dead {
            // TODO: Add thread reaping
            core.current_thread
                .store(ptr::null_mut(), Ordering::Release);
        } else if next_thread.is_some() {
            *current_thread.state.lock() = ThreadState::Idle;
            core.thread_queue.lock().push_back(current_thread);
        } else {
            let current_thread_ptr = Arc::as_ptr(&current_thread) as *mut Thread;
            core.current_thread
                .store(current_thread_ptr, Ordering::Release);
            mem::forget(current_thread);

            apic::send_eoi();
            unsafe { switch_process(&raw const (*current_thread_ptr).registers) };
        }
    }

    let Some(next_thread) = next_thread else {
        core.current_thread
            .store(ptr::null_mut(), Ordering::Release);

        let idle_thread = core.idle_thread.load(Ordering::Acquire);

        apic::send_eoi();
        unsafe { switch_process(&raw const (*idle_thread).registers) };
    };

    if current_thread.is_null()
        || unsafe { (&(*current_thread)).process.id != next_thread.process.id }
    {
        vmm::switch_to_page_map(next_thread.process.page_map);
    }

    let next_thread_ptr = Arc::as_ptr(&next_thread) as *mut Thread;
    unsafe {
        *(*next_thread_ptr).state.lock() = ThreadState::Busy;
        cpu::fxrstor(&mut (*next_thread_ptr).fx_state);
        cpu::set_fs_base((*next_thread_ptr).fs_base);
    }

    core.current_thread
        .store(next_thread_ptr, Ordering::Release);
    mem::forget(next_thread);

    apic::send_eoi();
    unsafe { switch_process(&raw const (*next_thread_ptr).registers) };
}
