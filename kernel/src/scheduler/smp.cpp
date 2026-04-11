/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "scheduler/smp.hpp"

#include "acpi/apic.hpp"
#include "arch/x86_64/cpu.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/idt.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "scheduler/scheduler.hpp"

namespace smp {

static u32 s_bsp_lapic_id { 0 };

static void cpu_init(limine_mp_info *info);

void initialize()
{
    const limine_mp_response *response = boot::get_mp_response();
    s_bsp_lapic_id = response->bsp_lapic_id;

    logger::debug("SMP: Found %u available CPUs\n", response->cpu_count);

    for (usize i { 0 }; i < response->cpu_count; ++i) {
        limine_mp_info *info = response->cpus[i];

        const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size + boot::get_hhdm_offset();
        Cpu::early_initialize(i, info->lapic_id, stack);

        info->extra_argument = i;

        if (info->lapic_id == s_bsp_lapic_id) {
            cpu_init(info);
            continue;
        }

        info->goto_address = cpu_init;
    }

    while (Cpu::online_count() != response->cpu_count) {
        asm volatile("");
    }

    logger::debug("SMP: Successfully started all %u CPUs\n", Cpu::online_count());

    logger::info("SMP: Initialized\n");
}

static void cpu_init(limine_mp_info *info)
{
    Cpu::disable_interrupts();

    idt::load();
    vmm::switch_to_page_map(vmm::get_kernel_page_map());

    Cpu &cpu = Cpu::get_from_id(static_cast<u32>(info->extra_argument));
    gdt::load(cpu.gdt(), cpu.tss());
    cpu.initialize();

    apic::enable_lapic();

    if (info->lapic_id == s_bsp_lapic_id) {
        return;
    }

    scheduler::yield();
}

} // namespace smp
