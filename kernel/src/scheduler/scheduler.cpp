/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "scheduler/scheduler.hpp"

#include "acpi/apic.hpp"
#include "arch/x86_64/cpu.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/registers.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "lib/atomic.hpp"
#include "lib/vector.hpp"
#include "memory/pmm.hpp"
#include "scheduler/process.hpp"
#include "scheduler/reaper.hpp"
#include "scheduler/thread.hpp"

namespace scheduler {

extern "C" [[noreturn]] void switch_process(const Registers *registers);

static Atomic<i32> s_current_process_id = 0;
static Atomic<i32> s_current_thread_id = 0;

static Process *s_kernel_process = nullptr;

static void schedule(const Registers &registers);

void initialize()
{
    idt::set_handler(0x20, schedule);

    s_kernel_process = create_process(vmm::get_kernel_page_map());
    logger::debug("Scheduler: Created kernel process with id #%u\n", s_kernel_process->id.get());

    logger::info("Scheduler: Initialized\n");
}

[[noreturn]] void yield()
{
    while (true) {
        cpu::enable_interrupts();
        cpu::halt();
    }
}

Process *create_process(vmm::PageMap *page_map)
{
    assert(page_map);

    const ProcessId id = ProcessId { s_current_process_id.fetch_add(1) };

    // TODO: Copy higher half of page map to always have the kernel mapped

    return new Process {
        .id = id,
        .state = Process::State::Idle,
        .page_map = page_map,
        .thread_list = nullptr,
    };
}

static void thread_exit()
{
    cpu::enter_critical();
    cpu::current().current_thread->state = Thread::State::Dead;
    cpu::leave_critical();

    yield();
}

static void thread_wrapper(void (*entry)())
{
    entry();
    thread_exit();
}

Thread *create_thread(Process *process, const u64 cs, void (*entry)())
{
    assert(process);
    assert(entry);

    const ThreadId id = ThreadId { s_current_thread_id.fetch_add(1) };
    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    const u64 virtual_stack = stack + (cs == 0x28 ? boot::get_hhdm_offset() : 0);

    vmm::map(
        process->page_map,
        stack - memory::s_page_size,
        virtual_stack - memory::s_page_size,
        vmm::Attribute::Write | (cs == 0x28 ? vmm::Attribute::None : vmm::Attribute::User));

    u64 *stack_ptr = reinterpret_cast<u64 *>(virtual_stack);
    if (cs == 0x28) {
        *--stack_ptr = reinterpret_cast<u64>(__builtin_return_address(0));
        *--stack_ptr = reinterpret_cast<u64>(reinterpret_cast<void *>(&create_thread));
    }

    Thread *thread = new Thread {
        .id = id,
        .state = Thread::State::Idle,
        .registers = {
            .rbp = reinterpret_cast<u64>(stack_ptr),
            .cs = cs,
            .flags = 1 << 9 | 1 << 1,
            .rsp = reinterpret_cast<u64>(stack_ptr),
        },
        .stack = reinterpret_cast<u8*>(stack),
        .stack_size = memory::s_page_size,
        .process = process,
        .next_thread = nullptr,
        .thread_list = nullptr,
    };

    if (cs == 0x28) {
        thread->registers.rdi = reinterpret_cast<u64>(entry);
        thread->registers.rip = reinterpret_cast<u64>(thread_wrapper);
        thread->registers.ss = thread->registers.cs + 0x08;
    } else {
        thread->registers.rip = reinterpret_cast<u64>(entry);
        thread->registers.ss = thread->registers.cs - 0x08;
    }

    Thread *thread_list = process->thread_list;
    if (!thread_list) {
        process->thread_list = thread;
    } else {
        while (thread_list->thread_list != nullptr) {
            thread_list = thread_list->thread_list;
        }
        thread_list->thread_list = thread;
    }

    // FIXME: Add load-balancing

    usize least_load = 0xffffffffffffffff;
    cpu::Core *least_loaded_cpu = nullptr;
    cpu::for_each([&](cpu::Core &core) {
        const usize load = core.thread_count;

        if (load < least_load) {
            least_load = load;
            least_loaded_cpu = &core;
        }
    });

    if (!least_loaded_cpu->next_thread) {
        least_loaded_cpu->next_thread = thread;
    } else {
        Thread *current_thread = least_loaded_cpu->next_thread;
        while (current_thread->next_thread != nullptr) {
            current_thread = current_thread->next_thread;
        }

        current_thread->next_thread = thread;
    }

    least_loaded_cpu->thread_count += 1;

    return thread;
}

Thread *create_idle_thread()
{
    const ThreadId id = ThreadId { s_current_thread_id.fetch_add(1) };

    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    constexpr u64 cs = 0x28;

    Thread *thread = new Thread {
        .id = id,
        .state = Thread::State::Idle,
        .registers = {
            .rbp = 0,
            // NOTE: Iret Frame
            .rip = reinterpret_cast<u64>(yield),
            .cs = cs,
            .flags = 1 << 9 | 1 << 1,
            .rsp = stack + boot::get_hhdm_offset(),
            .ss = cs + 0x08,
        },
        .stack = reinterpret_cast<u8*>(stack),
        .stack_size = memory::s_page_size,
        .process = s_kernel_process,
        .next_thread = nullptr,
    };

    Thread *thread_list = s_kernel_process->thread_list;
    if (!thread_list) {
        s_kernel_process->thread_list = thread;
    } else {
        while (thread_list->thread_list != nullptr) {
            thread_list = thread_list->thread_list;
        }
        thread_list->thread_list = thread;
    }

    return thread;
}

void destroy_thread(ThreadId) { }

// TODO: Add create thread from current process

Process *get_kernel_process() { return s_kernel_process; }

// TODO: Handle threads which are in a Dead state
// TODO: If queue is empty, then replace with idle thread, because it does not get pushed back

void schedule(const Registers &registers)
{
    apic::send_eoi();

    cpu::Core &core = cpu::current();

    Thread *previous_thread = core.current_thread;

    if (previous_thread && previous_thread->state == Thread::State::Dead) {
        // NOTE: We add the thread to reap
        reaper::add_thread_to_reap(previous_thread);
        previous_thread = nullptr;
        core.current_thread = nullptr;
    }

    if (!core.next_thread) {
        // NOTE: If there is a thread running and no thread queued, then continue running
        if (previous_thread) {
            return;
        }

        // NOTE: If there is no thread running and no thread queued, then idle
        switch_process(&core.idle_thread->registers);
    }

    // NOTE: Push thread back to the end
    if (previous_thread) {
        Thread *thread = core.next_thread;
        while (thread->next_thread != nullptr) {
            thread = thread->next_thread;
        }

        thread->next_thread = previous_thread;
    }

    Thread *next_thread = core.next_thread;

    core.current_thread = next_thread;
    core.next_thread = next_thread->next_thread;

    next_thread->next_thread = nullptr;

    if (previous_thread) {
        previous_thread->registers = registers;
        previous_thread->state = Thread::State::Idle;
    }

    if (!previous_thread || previous_thread->process != next_thread->process) {
        vmm::switch_to_page_map(next_thread->process->page_map);
    }

    next_thread->state = Thread::State::Busy;

    switch_process(&next_thread->registers);
}

} // namespace scheduler
