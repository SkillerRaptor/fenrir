/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscall/syscalls/exit.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/logger.hpp"
#include "scheduler/scheduler.hpp"

namespace syscalls {

// FIXME: This should kill every thread in the process

void sys$exit(const SyscallRegisters *)
{
    cpu::enter_critical();
    cpu::current().current_thread->state = Thread::State::Dead;

    cpu::leave_critical();

    scheduler::yield();
}

} // namespace syscalls
