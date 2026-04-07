/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "memory/vmm.hpp"
#include "scheduler/types.hpp"

namespace scheduler {

void initialize();

[[noreturn]] void yield();

ProcessId create_process(vmm::PageMap *);

ThreadId create_thread(ProcessId, u64 cs, void (*entry)());
ThreadId create_idle_thread();

ProcessId get_kernel_process();

} // namespace scheduler
