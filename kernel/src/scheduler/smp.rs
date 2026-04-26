//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::sync::atomic::{AtomicU8, Ordering};

use limine::mp::MpInfo;

use crate::{
    acpi::apic,
    arch::x86_64::{cpu, gdt, idt},
    common::boot,
    memory::vmm,
};

static ONLINE_COUNT: AtomicU8 = AtomicU8::new(1);

pub fn initialize() {
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

extern "C" fn core_init(info: &MpInfo) -> ! {
    cpu::disable_interrupts();

    gdt::load();
    idt::load();
    vmm::switch_to_kernel_page_map();

    apic::enable_lapic();

    log::debug!("Started Core #{} successfully", info.extra_argument());
    ONLINE_COUNT.fetch_add(1, Ordering::Release);

    cpu::hcf();
}
