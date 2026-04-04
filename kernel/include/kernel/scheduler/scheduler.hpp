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

ProcessId create_process();
ThreadId create_thread(ProcessId pid, void (*entry)(void *), void *user_argument);
ThreadId create_idle_thread();

ProcessId get_kernel_process();

} // namespace kernel::scheduler
