/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.hpp"

#include "acpi/apic.hpp"
#include "lib/assert.hpp"
#include "lib/atomic.hpp"
#include "scheduler/scheduler.hpp"

namespace cpu {

// FIXME: Add config header to change this value
static constexpr usize s_max_cores = 16;

static Core s_cores[s_max_cores] { };
static Atomic<u32> s_online_cores = 0;

void early_initialize(const u32 id, const u32 lapic_id, const u64 kernel_stack)
{
    Core &core = s_cores[id];

    core.id = id;
    core.lapic_id = lapic_id;
    core.kernel_rsp = kernel_stack;
    core.user_rsp = 0;

    core.tss.rsp_0 = kernel_stack;
    core.gdt.table = gdt::create_table();
    core.gdt.descriptor = { };

    core.idle_thread = scheduler::create_idle_thread();
    core.current_thread = nullptr;
    core.next_thread = nullptr;
}

void initialize(const u32 id)
{
    set_gs_base(&s_cores[id]);
    set_kernel_gs_base(&s_cores[id]);
}

void add_online() { s_online_cores.fetch_add(1); }

Core &current()
{
    u32 cpu_id = 0;
    asm volatile("mov %%gs:0x00, %0" : "=r"(cpu_id));
    return s_cores[cpu_id];
}

Core &by_id(const u32 id) { return s_cores[id]; }

u32 online_count() { return s_online_cores.load(); }

void enter_critical()
{
    const bool were_interrupts_enabled = are_interrupts_enabled();
    disable_interrupts();

    // NOTE: We first disable interrupts, so we don't get any corruption state
    Core &core = current();
    core.critical_sections += 1;
    core.were_interrupts_enabled = were_interrupts_enabled;
}

void leave_critical()
{
    Core &core = current();
    assert(core.critical_sections > 0);

    core.critical_sections -= 1;
    if (core.were_interrupts_enabled) {
        core.were_interrupts_enabled = false;
        enable_interrupts();
    }
}

bool is_in_critical() { return current().critical_sections > 0; }

} // namespace cpu
