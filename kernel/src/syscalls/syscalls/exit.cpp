/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.hpp"
#include "scheduler/scheduler.hpp"
#include "syscall/syscalls.hpp"

namespace syscalls {

// FIXME: This should kill every thread in the process

u64 sys$exit(const Span<const u64>)
{
    cpu::enter_critical();
    cpu::current().current_thread->state = Thread::State::Dead;

    cpu::leave_critical();

    scheduler::yield();
}

} // namespace syscalls
