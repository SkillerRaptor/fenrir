/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "sync/spinlock.hpp"

#include "arch/x86_64/cpu.hpp"
#include "lib/assert.hpp"

// FIXME: Somehow the check for interrupts results in a page fault, find out why

void Spinlock::lock()
{
    assert(!Cpu::are_interrupts_enabled());

    while (atomic_flag_test_and_set_explicit(&m_lock, memory_order_acquire)) {
        Cpu::pause();
    }
}

void Spinlock::unlock()
{
    assert(!Cpu::are_interrupts_enabled());

    atomic_flag_clear_explicit(&m_lock, memory_order_release);
}
