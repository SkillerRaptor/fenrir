/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/scheduler/process.hpp"
#include "kernel/scheduler/thread.hpp"

namespace kernel::scheduler {

void initialize();

void yield();

Process::Id create_process();
Thread::Id create_thread(Process::Id pid, void (*function)(void *), void *user_argument);
Thread::Id create_idle_thread();

Process::Id get_kernel_process();

} // namespace kernel::scheduler
