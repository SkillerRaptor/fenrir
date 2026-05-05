//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::sync::Arc;
use core::sync::atomic::{AtomicU8, Ordering};

use limine::mp::MpInfo;

use crate::{
    acpi::apic,
    arch::x86_64::{
        cpu::{self, Core},
        gdt,
        idt,
    },
    common::boot,
    memory::vmm,
    scheduler,
};

static ONLINE_COUNT: AtomicU8 = AtomicU8::new(1);

pub fn initialize() {
    cpu::initialize_cores();

    let mp_infos = boot::get_mp_infos();
    log::debug!("SMP: Found {} available cores", mp_infos.len());

    for (i, info) in mp_infos.iter().enumerate() {
        if i as u32 == boot::get_bsp_lapic_id() {
            continue;
        }

        info.bootstrap(core_init, i as u64);
    }

    while ONLINE_COUNT.load(Ordering::Acquire) != mp_infos.len() as u8 {}

    log::debug!(
        "SMP: Successfully started all {} cores",
        ONLINE_COUNT.load(Ordering::Relaxed)
    );

    log::info!("SMP: Initialized");
}

fn thread_idle() {
    loop {
        cpu::enable_interrupts();
        cpu::pause();
    }
}

extern "C" fn core_init(info: &MpInfo) -> ! {
    cpu::disable_interrupts();

    gdt::load();
    idt::load();
    vmm::switch_to_page_map(vmm::get_kernel_page_map());

    cpu::set_current_core(Core::by_id(info.extra_argument() as usize));

    let core = Core::current();
    core.idle_thread.store(
        Arc::into_raw(scheduler::create_kernel_thread(thread_idle)) as *mut _,
        Ordering::Relaxed,
    );

    apic::enable_lapic();

    ONLINE_COUNT.fetch_add(1, Ordering::Release);

    scheduler::reschedule();
}
