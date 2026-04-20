/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "memory/vmm.hpp"

struct Process;
struct Thread;

namespace scheduler {

void initialize();

[[noreturn]] void yield();

Process *create_process(vmm::PageMap *);

Thread *create_kernel_thread(void (*entry)());
Thread *create_user_thread(Process *, void (*entry)());
Thread *create_idle_thread();

Process *get_kernel_process();

} // namespace scheduler
