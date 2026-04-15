/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "scheduler/reaper.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "memory/pmm.hpp"
#include "scheduler/process.hpp"
#include "sync/spinlock.hpp"

namespace reaper {

static Thread *s_next_threads { nullptr };
static Spinlock s_lock { };

[[noreturn]] void run()
{
    while (true) {
        if (s_next_threads == nullptr) {
            continue;
        }

        cpu::enter_critical();
        s_lock.lock();

        Thread *reaping_thread = s_next_threads;
        s_next_threads = s_next_threads->next_thread;

        Process *process = reaping_thread->process;

        if (process->thread_list->id == reaping_thread->id) {
            process->thread_list = reaping_thread->thread_list;
        } else {
            Thread *previous_thread = process->thread_list;
            while (previous_thread && previous_thread->thread_list != reaping_thread) {
                previous_thread = previous_thread->thread_list;
            }

            if (previous_thread) {
                previous_thread->thread_list = reaping_thread->thread_list;
            }
        }

        reaping_thread->thread_list = nullptr;

        if (!process->thread_list) {
            vmm::destroy_page_map(process->page_map);
            delete process;
        }

        pmm::free(reaping_thread->stack, reaping_thread->stack_size / memory::s_page_size);
        delete reaping_thread;

        s_lock.unlock();
        cpu::leave_critical();
    }
}

void add_thread_to_reap(Thread *thread)
{
    cpu::enter_critical();
    s_lock.lock();

    if (!s_next_threads) {
        s_next_threads = thread;
    } else {
        Thread *current = s_next_threads;
        while (current->next_thread != nullptr) {
            current = current->next_thread;
        }

        current->next_thread = thread;
    }

    s_lock.unlock();

    cpu::leave_critical();
}

} // namespace reaper
