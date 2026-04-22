/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "acpi/hpet.hpp"

#include <uacpi/acpi.h>
#include <uacpi/tables.h>

#include "core/boot.hpp"
#include "core/logger.hpp"
#include "lib/assert.hpp"
#include "lib/types.hpp"
#include "memory/mmio.hpp"
#include "memory/vmm.hpp"

namespace hpet {

static constexpr u64 s_general_capabilities_register = 0x000;
static constexpr u64 s_general_configuration_register = 0x010;
static constexpr u64 s_main_counter_register = 0x0f0;

static u64 s_virtual_address = 0;
static u32 s_clock_period = 0;

void initialize()
{
    uacpi_table table { };
    const uacpi_status ret = uacpi_table_find_by_signature(ACPI_HPET_SIGNATURE, &table);
    if (uacpi_unlikely_error(ret)) {
        logger::error("uacpi_table_find_by_signature error: {}\n", uacpi_status_to_string(ret));
    }

    const acpi_hpet *hpet = static_cast<acpi_hpet *>(table.ptr);
    const u64 physical_address = hpet->address.address;

    uacpi_table_unref(&table);

    vmm::map(
        vmm::get_kernel_page_map(),
        physical_address,
        physical_address + boot::get_hhdm_offset(),
        vmm::Attribute::Write);

    s_virtual_address = physical_address + boot::get_hhdm_offset();
    logger::debug("HPET: Mapping MMIO {:#016x} -> {:#016x}\n", physical_address, s_virtual_address);

    s_clock_period = mmio::in<u32>(s_virtual_address + s_general_capabilities_register + 0x04);
    logger::debug("HPET: Clock period configured with {}ns\n", s_clock_period / 1000000);

    // NOTE: Reset

    mmio::out<u64>(
        s_virtual_address + s_general_configuration_register,
        mmio::in<u64>(s_virtual_address + s_general_configuration_register) & ~(1ull << 0));
    mmio::out<u64>(s_virtual_address + s_main_counter_register, 0);
    mmio::out<u64>(
        s_virtual_address + s_general_configuration_register,
        mmio::in<u64>(s_virtual_address + s_general_configuration_register) | 0b1);
    logger::debug("HPET: Reset and enabled timer\n");

    logger::info("HPET: Initialized\n");
}

void sleep(const u64 ms)
{
    assert(ms > 0);

    const u64 target_ticks
        = mmio::in<u64>(s_virtual_address + s_main_counter_register) + (ms * 1000000000000) / s_clock_period;
    while (mmio::in<u64>(s_virtual_address + s_main_counter_register) < target_ticks) { }
}

} // namespace hpet
