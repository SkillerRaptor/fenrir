/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "acpi/apic.hpp"

#include "acpi/hpet.hpp"
#include "arch/x86_64/cpu.hpp"
#include "arch/x86_64/pic.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "lib/types.hpp"
#include "memory/mmio.hpp"
#include "memory/vmm.hpp"

namespace apic {

static constexpr u32 s_apic_base_msr = 0x1b;

static constexpr u64 s_spurious_interrupt_vector_register = 0x0f0;
static constexpr u64 s_end_of_interrupt_register = 0x0b0;

// NOTE: LVT
static constexpr u64 s_timer_register = 0x320;
static constexpr u64 s_timer_initial_count_register = 0x380;
static constexpr u64 s_timer_current_count_register = 0x390;
static constexpr u64 s_timer_divide_configuration_register = 0x3e0;

// NOTE: LVT Register Format
static constexpr u32 s_register_mask = 1 << 16;

static constexpr u32 s_timer_isr = 0x20;
static constexpr u32 s_timer_divide_value = 0b011; // NOTE: Divide by 16
static constexpr u32 s_timer_periodic_mode = 0b01 << 17;

static u64 s_base_lapic_address = 0;

void initialize()
{
    pic::disable();

    // TODO: Implement I/O APIC

    const u32 lapic_physical_address = cpu::read_msr(s_apic_base_msr) & 0xfffff000;
    s_base_lapic_address = lapic_physical_address + boot::get_hhdm_offset();

    logger::debug("APIC: Mapping LAPIC MMIO 0x%016llx -> 0x%016llx\n", lapic_physical_address, s_base_lapic_address);
    vmm::map(vmm::get_kernel_page_map(), lapic_physical_address, s_base_lapic_address, vmm::Attribute::Write);

    constexpr u32 apic_global_enable = 1 << 11;
    cpu::write_msr(s_apic_base_msr, cpu::read_msr(s_apic_base_msr) | apic_global_enable);

    enable_lapic();

    logger::debug("APIC: Enabled LAPIC Timer\n");

    logger::info("APIC: Initialized\n");
}

void enable_lapic()
{
    constexpr u32 apic_software_enable = 1 << 8;
    constexpr u32 spurious_vector = 0xff;
    mmio::out<u32>(
        s_base_lapic_address + s_spurious_interrupt_vector_register,
        mmio::in<u32>(s_base_lapic_address + s_spurious_interrupt_vector_register) | apic_software_enable
            | spurious_vector);

    // NOTE: Calibrate timer
    mmio::out<u32>(s_base_lapic_address + s_timer_divide_configuration_register, s_timer_divide_value);

    // NOTE: Set initial counter value to -1
    mmio::out<u32>(s_base_lapic_address + s_timer_initial_count_register, 0xffffffff);

    // NOTE: Sleep for 10ms to calibrate the timer
    hpet::sleep(10);

    mmio::out<u32>(s_base_lapic_address + s_timer_register, s_register_mask);

    const u32 ticks = 0xffffffff - mmio::in<u32>(s_base_lapic_address + s_timer_current_count_register);
    mmio::out<u32>(s_base_lapic_address + s_timer_register, s_timer_periodic_mode | s_timer_isr);
    mmio::out<u32>(s_base_lapic_address + s_timer_divide_configuration_register, s_timer_divide_value);
    mmio::out<u32>(s_base_lapic_address + s_timer_initial_count_register, ticks);
}

void send_eoi() { mmio::out<u32>(s_base_lapic_address + s_end_of_interrupt_register, 0); }

void send_ipi(const u32 lapic_id, const u8 vector)
{
    if (lapic_id == 0xff) {
        // NOTE: This is a hack to send to every other core except self
        mmio::out<u32>(s_base_lapic_address + 0x310, 0);
        mmio::out<u32>(s_base_lapic_address + 0x300, 1 << 19 | 1 << 18 | vector);
    }
}

} // namespace apic
