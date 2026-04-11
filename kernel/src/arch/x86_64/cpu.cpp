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

// FIXME: Add config header to change this value
static constexpr usize s_max_cpus = 16;

static Cpu s_cpus[s_max_cpus] { };
static Atomic<u32> s_online_cpus { 0 };

void Cpu::early_initialize(const u32 id, const u32 lapic_id, const u64 kernel_stack)
{
    Cpu &cpu = s_cpus[id];
    assert(!cpu.m_is_initialized);

    cpu.m_id = id;
    cpu.m_lapic_id = lapic_id;
    cpu.m_kernel_rsp = kernel_stack;
    cpu.m_user_rsp = 0;

    cpu.m_tss.rsp_0 = kernel_stack;
    cpu.m_gdt.table = gdt::create_table();
    cpu.m_gdt.descriptor = { };

    cpu.m_idle_thread = scheduler::create_idle_thread();
    cpu.m_current_thread = nullptr;
    cpu.m_next_thread = nullptr;

    cpu.m_is_initialized = true;
}

void Cpu::initialize()
{
    cpu::set_gs_base(this);
    cpu::set_kernel_gs_base(this);

    s_online_cpus.fetch_add(1);
}

Cpu &Cpu::current()
{
    u32 cpu_id = 0;
    asm volatile("mov %%gs:0x00, %0" : "=r"(cpu_id));
    return s_cpus[cpu_id];
}

Cpu &Cpu::get_from_id(const u32 id) { return s_cpus[id]; }

u32 Cpu::online_count() { return s_online_cpus.load(); }

void Cpu::enter_critical_section()
{
    if (m_critical_sections++ == 0 && are_interrupts_enabled()) {
        m_were_interrupts_enabled = true;
        disable_interrupts();
    }
}

void Cpu::leave_critical_section()
{
    assert(m_critical_sections > 0);
    if (--m_critical_sections == 0 && m_were_interrupts_enabled) {
        enable_interrupts();
    }
}

bool Cpu::is_in_critical_section() const { return m_critical_sections > 0; }
