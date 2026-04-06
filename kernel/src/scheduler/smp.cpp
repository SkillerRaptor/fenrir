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
#include "sync/spinlock.hpp"

namespace kernel::smp {

static u32 s_bsp_lapic_id = 0;
static u8 s_online_cpu_count = 0;
static cpu::Info *s_cpu_infos = nullptr;
static Spinlock s_spinlock {};

static void cpu_init(limine_mp_info *info);

void initialize()
{
    const limine_mp_response *response = boot::get_mp_response();
    s_bsp_lapic_id = response->bsp_lapic_id;
    s_cpu_infos = new cpu::Info[response->cpu_count];

    logger::debug("SMP: Found %u available CPUs\n", response->cpu_count);

    for (usize i = 0; i < response->cpu_count; ++i) {
        limine_mp_info *info = response->cpus[i];

        const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size + boot::get_hhdm_offset();

        s_cpu_infos[i].id = i;
        s_cpu_infos[i].user_rsp = 0;
        s_cpu_infos[i].kernel_rsp = stack;
        s_cpu_infos[i].lapic_id = info->lapic_id;
        s_cpu_infos[i].current_thread = ThreadId { -1 };
        s_cpu_infos[i].idle_thread = ThreadId { -1 };
        s_cpu_infos[i].run_queue = {};
        s_cpu_infos[i].run_queue_lock = {};
        s_cpu_infos[i].tss = {};
        s_cpu_infos[i].tss.rsp_0 = stack;
        s_cpu_infos[i].gdt.table = gdt::create_table();
        s_cpu_infos[i].gdt.descriptor = {};

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

    logger::debug("SMP: Successfully started all %u CPUs\n", s_online_cpu_count);

    logger::info("SMP: Initialized\n");
}

cpu::Info *get_cpu_infos() { return s_cpu_infos; }

static void cpu_init(limine_mp_info *info)
{
    cpu::disable_interrupts();

    idt::load();
    vmm::switch_to_page_map(vmm::get_kernel_page_map());

    cpu::Info *cpu_info = reinterpret_cast<cpu::Info *>(info->extra_argument);
    gdt::load(cpu_info->gdt, cpu_info->tss);
    cpu::set_gs_base(cpu_info);
    cpu::set_kernel_gs_base(cpu_info);

    cpu_info->idle_thread = scheduler::create_idle_thread();

    apic::enable_lapic();

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
