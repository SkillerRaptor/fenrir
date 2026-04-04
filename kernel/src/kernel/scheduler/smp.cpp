/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/scheduler/smp.hpp"

#include "kernel/acpi/apic.hpp"
#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/gdt.hpp"
#include "kernel/arch/x86_64/idt.hpp"
#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/memory/vmm.hpp"
#include "kernel/scheduler/scheduler.hpp"
#include "kernel/sync/spinlock.hpp"

namespace kernel::smp {

static u32 s_bsp_lapic_id = 0;
static u8 s_online_cpu_count = 0;
static cpu::Info *s_cpu_infos = nullptr;
static Spinlock s_spinlock { };

static void cpu_init(limine_mp_info *info);

void initialize()
{
    const limine_mp_response *response = boot::get_mp_response();
    s_bsp_lapic_id = response->bsp_lapic_id;
    s_cpu_infos = new cpu::Info[response->cpu_count];

    logger::debug("SMP: Found %u available CPUs\n", response->cpu_count);

    for (usize i = 0; i < response->cpu_count; ++i) {
        limine_mp_info *info = response->cpus[i];

        s_cpu_infos[i].id = i;
        s_cpu_infos[i].lapic_id = info->lapic_id;
        s_cpu_infos[i].current_thread = -1;
        s_cpu_infos[i].idle_thread = -1;
        s_cpu_infos[i].run_queue = { };
        s_cpu_infos[i].run_queue_lock = { };

        info->extra_argument = reinterpret_cast<u64>(&s_cpu_infos[i]);

        if (info->lapic_id == s_bsp_lapic_id) {
            cpu_init(info);
            continue;
        }

        info->goto_address = cpu_init;
    }

    while (s_online_cpu_count != response->cpu_count) {
        asm volatile("");
    }

    logger::info("SMP: Initialized\n");
}

cpu::Info *get_cpu_infos() { return s_cpu_infos; }

static void cpu_init(limine_mp_info *info)
{
    cpu::disable_interrupts();

    gdt::load();
    idt::load();
    vmm::switch_to_page_map(vmm::get_kernel_page_map());

    cpu::Info *cpu_info = reinterpret_cast<cpu::Info *>(info->extra_argument);
    cpu::set_gs_base(cpu_info);

    cpu_info->idle_thread = scheduler::create_idle_thread();

    apic::enable_lapic();

    logger::debug("SMP: Started CPU #%u\n", cpu_info->id);

    {
        SpinlockLocker _locker(s_spinlock);
        ++s_online_cpu_count;
    }

    if (info->lapic_id == s_bsp_lapic_id) {
        return;
    }

    scheduler::yield();
}

} // namespace kernel::smp
