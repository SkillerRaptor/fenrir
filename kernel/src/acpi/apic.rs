//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use crate::{
    acpi::hpet,
    arch::x86_64::{cpu, pic},
    common::{boot, mmio, once::Once},
    memory::vmm::{self, Attribute},
};

const APIC_BASE_MSR: u32 = 0x1b;

const SPURIOUS_INTERRUPT_VECTOR_REGISTER: u64 = 0x0f0;
const END_OF_INTERRUPT_REGISTER: u64 = 0x0b0;

// NOTE: LVT
const TIMER_REGISTER: u64 = 0x320;
const TIMER_INITIAL_COUNTER_REGISTER: u64 = 0x380;
const TIMER_CURRENT_COUNT_REGISTER: u64 = 0x390;
const TIMER_DIVIDE_CONFIGURATION_REGISTER: u64 = 0x3e0;

// NOTE: LVT Register Format
const REGISTER_MASK: u32 = 1 << 16;

pub const TIMER_ISR: u32 = 0x20;
const TIMER_DIVIDE_VALUE: u32 = 0b011; // NOTE: Divide by 16
const TIMER_PERIODIC_MODE: u32 = 0b01 << 17;

static BASE_LAPIC_ADDRESS: Once<u64> = Once::new();

pub fn initialize() {
    pic::disable();

    let lapic_physical_address = cpu::read_msr(APIC_BASE_MSR) & 0xfffff000;

    unsafe {
        BASE_LAPIC_ADDRESS.initialize(lapic_physical_address + boot::get_hhdm_offset());
    }

    vmm::map_into_kernel(lapic_physical_address, Attribute::WRITE);

    log::debug!(
        "HPET: Mapping MMIO {:#018x} -> {:#018x}",
        lapic_physical_address,
        BASE_LAPIC_ADDRESS.get()
    );

    const APIC_GLOBAL_ENABLE: u64 = 1 << 11;
    cpu::write_msr(
        APIC_BASE_MSR,
        cpu::read_msr(APIC_BASE_MSR) | APIC_GLOBAL_ENABLE,
    );

    enable_lapic();

    log::debug!("APIC: Enabled LAPIC timer");

    log::info!("APIC: Initialized");
}

pub fn enable_lapic() {
    const APIC_SOFTWARE_ENABLE: u32 = 1 << 8;
    const SPURIOUS_VECTOR: u32 = 0xff;

    unsafe {
        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + SPURIOUS_INTERRUPT_VECTOR_REGISTER,
            mmio::read::<u32>(BASE_LAPIC_ADDRESS.get() + SPURIOUS_INTERRUPT_VECTOR_REGISTER)
                | APIC_SOFTWARE_ENABLE
                | SPURIOUS_VECTOR,
        );

        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + TIMER_DIVIDE_CONFIGURATION_REGISTER,
            TIMER_DIVIDE_VALUE,
        );

        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + TIMER_INITIAL_COUNTER_REGISTER,
            0xffffffff,
        );
    }

    hpet::sleep(20);

    unsafe {
        mmio::write::<u32>(BASE_LAPIC_ADDRESS.get() + TIMER_REGISTER, REGISTER_MASK);

        let ticks =
            0xffffffff - mmio::read::<u32>(BASE_LAPIC_ADDRESS.get() + TIMER_CURRENT_COUNT_REGISTER);
        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + TIMER_REGISTER,
            TIMER_PERIODIC_MODE | TIMER_ISR,
        );
        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + TIMER_DIVIDE_CONFIGURATION_REGISTER,
            TIMER_DIVIDE_VALUE,
        );
        mmio::write::<u32>(
            BASE_LAPIC_ADDRESS.get() + TIMER_INITIAL_COUNTER_REGISTER,
            ticks,
        );
    }
}

pub fn send_eoi() {
    unsafe {
        mmio::write::<u32>(BASE_LAPIC_ADDRESS.get() + END_OF_INTERRUPT_REGISTER, 0);
    }
}
