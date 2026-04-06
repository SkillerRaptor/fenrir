/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "sync/spinlock.hpp"

#include "arch/x86_64/cpu.hpp"

namespace kernel {

void Spinlock::lock()
{
    while (atomic_flag_test_and_set_explicit(&m_lock, memory_order_acquire)) {
        cpu::pause();
    }
}

void Spinlock::unlock() { atomic_flag_clear_explicit(&m_lock, memory_order_release); }

} // namespace kernel
