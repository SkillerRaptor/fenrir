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
#include "syscall/syscalls.hpp"

namespace smp {

static u32 s_bsp_lapic_id = 0;

static void cpu_init(limine_mp_info *info);

void initialize()
{
    const limine_mp_response *response = boot::get_mp_response();
    s_bsp_lapic_id = response->bsp_lapic_id;

    logger::debug("SMP: Found {} available CPUs\n", response->cpu_count);

    for (usize i = 0; i < response->cpu_count; ++i) {
        limine_mp_info *info = response->cpus[i];

        const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size + boot::get_hhdm_offset();
        cpu::early_initialize(i, info->lapic_id, stack);

        info->extra_argument = i;

        if (info->lapic_id == s_bsp_lapic_id) {
            cpu_init(info);
            continue;
        }

        info->goto_address = cpu_init;
    }

    while (cpu::online_count() != response->cpu_count) {
        asm volatile("");
    }

    logger::debug("SMP: Successfully started all {} CPUs\n", cpu::online_count());

    logger::info("SMP: Initialized\n");
}

static void cpu_init(limine_mp_info *info)
{
    cpu::disable_interrupts();

    idt::load();
    vmm::switch_to_page_map(vmm::get_kernel_page_map());

    cpu::Core &core = cpu::by_id(static_cast<u32>(info->extra_argument));
    gdt::load(core.gdt, core.tss);
    cpu::initialize(info->extra_argument);
    syscalls::load();

    apic::enable_lapic();

    cpu::add_online();

    if (info->lapic_id == s_bsp_lapic_id) {
        return;
    }

    scheduler::yield();
}

} // namespace smp
