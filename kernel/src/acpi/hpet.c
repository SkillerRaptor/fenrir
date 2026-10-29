/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "acpi/hpet.h"

#include <uacpi/acpi.h>
#include <uacpi/status.h>
#include <uacpi/tables.h>

#include "boot/boot.h"
#include "lib/assertion.h"
#include "lib/log.h"
#include "lib/mmio.h"
#include "mem/vmm.h"

static constexpr uint64_t GENERAL_CAPABILITIES_REGISTER = 0x000;
static constexpr uint64_t GENERAL_CONFIGURATION_REGISTER = 0x010;
static constexpr uint64_t MAIN_COUNTER_REGISTER = 0x0f0;

static uint64_t s_virtual_address = 0;
static uint32_t s_clock_period = 0;

static uint64_t read(uint64_t offset) { return mmio_read64((void *) (s_virtual_address + offset)); }

static void write(uint64_t offset, uint64_t value) { mmio_write64((void *) (s_virtual_address + offset), value); }

void hpet_init()
{
    uacpi_table table = { };
    const uacpi_status ret = uacpi_table_find_by_signature("HPET", &table);
    if (uacpi_unlikely_error(ret)) {
        PANIC("HPET: uacpi_table_find_by_signature failed: %s\n", uacpi_status_to_string(ret));
    }

    struct acpi_hpet *hpet = table.ptr;

    const uint64_t physical_address = hpet->address.address;
    vmm_map_page_into_kernel(physical_address, PAGE_ATTRIBUTE_WRITE);

    uacpi_table_unref(&table);

    s_virtual_address = physical_address + boot_get_hhdm_offset();
    LOG_DEBUG("HPET: Mapping MMIO %#018lx -> %#018lx\n", physical_address, s_virtual_address);

    s_clock_period = (uint32_t) (read(GENERAL_CAPABILITIES_REGISTER) >> 32 & 0xffffffff);
    LOG_DEBUG("HPET: Clock period configured with %uns\n", s_clock_period / 1000000);

    write(GENERAL_CONFIGURATION_REGISTER, read(GENERAL_CONFIGURATION_REGISTER) & ~0b1);
    write(MAIN_COUNTER_REGISTER, 0);
    write(GENERAL_CONFIGURATION_REGISTER, read(GENERAL_CONFIGURATION_REGISTER) | 0b1);

    LOG_DEBUG("HPET: Reset and enabled timer\n");

    LOG_INFO("HPET: Initialized\n");
}

void hpet_sleep(uint64_t ms)
{
    const uint64_t target_ticks = read(MAIN_COUNTER_REGISTER) + (ms * 1000000000000) / s_clock_period;

    while (read(MAIN_COUNTER_REGISTER) < target_ticks) { }
}
