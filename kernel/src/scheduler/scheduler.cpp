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
#include "scheduler/thread.hpp"

// TODO: Add reaper thread
// It should find every thread in a dead state, free the stack and remove it from the global list
// It should find every process, that has no thread anymore that is alive, free the lower half of the page map and
// remove it from the global list

namespace scheduler {

extern "C" [[noreturn]] void switch_process(const Registers *registers);

static Atomic<i32> s_current_process_id { 0 };
static Atomic<i32> s_current_thread_id { 0 };

static Process *s_kernel_process { nullptr };

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
    };
}

static void thread_wrapper(void (*entry)())
{
    entry();

    // TODO: Find a better way to get the current thread id
    // NOTE: Maybe pass the thread id as argument or make the thread blocking at the time
    // NOTE: The current thread id could change mid way and give the wrong result, resulting in a race condition

    const cpu::Info &current_cpu = cpu::get_local_cpu_info();
    current_cpu.next_thread[0].state = Thread::State::Dead;

    yield();
}

Thread *create_thread(Process *process, const u64 cs, void (*entry)())
{
    assert(process);
    assert(entry);

    const ThreadId id = ThreadId { s_current_thread_id.fetch_add(1) };
    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    Thread *thread = new Thread {
        .id = id,
        .state = Thread::State::Idle,
        .registers = {
            .cs = cs,
            .flags = 1 << 9 | 1 << 1,
            .rsp = stack,
        },
        .stack = reinterpret_cast<u8*>(stack),
        .stack_size = memory::s_page_size,
        .process = process,
        .next_thread = nullptr,
    };

    if (cs == 0x28) {
        thread->registers.rdi = reinterpret_cast<u64>(entry),
        thread->registers.rip = reinterpret_cast<u64>(thread_wrapper);
        thread->registers.rsp += boot::get_hhdm_offset();
        thread->registers.ss = thread->registers.cs + 0x08;
    } else {
        thread->registers.rip = reinterpret_cast<u64>(entry);
        thread->registers.ss = thread->registers.cs - 0x08;
    }

    vmm::map(
        process->page_map,
        stack - memory::s_page_size,
        thread->registers.rsp - memory::s_page_size,
        vmm::Attribute::Write | (cs == 0x28 ? vmm::Attribute::None : vmm::Attribute::User));

    // FIXME: Add load-balancing

    /*
    usize least_loaded_cpu = 0;
    usize least_load = 0xffffffffffffffff;
    cpu::Info *infos = smp::get_cpu_infos();
    for (usize i = 0; i < boot::get_mp_response()->cpu_count; ++i) {
        const usize load = infos[i].run_queue.size();

        if (load < least_load) {
            least_load = load;
            least_loaded_cpu = i;
        }
    }
    */

    // NOTE: This will always push the thread to the first CPU
    cpu::Info *infos = smp::get_cpu_infos();

    if (!infos[0].next_thread) {
        infos[0].next_thread = thread;
    } else {
        Thread *current_thread = infos[0].next_thread;
        while (current_thread->next_thread != nullptr) {
            current_thread = current_thread->next_thread;
        }

        current_thread->next_thread = thread;
    }

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

    cpu::Info &cpu = cpu::get_local_cpu_info();

    if (!cpu.next_thread) {
        // NOTE: If there is a thread running and no thread queued, then continue running
        if (cpu.current_thread) {
            return;
        }

        // NOTE: If there is no thread running and no thread queued, then idle
        switch_process(&cpu.idle_thread->registers);
    }

    Thread *previous_thread = cpu.current_thread;

    // NOTE: Push thread back to the end
    if (previous_thread) {
        Thread *thread = cpu.next_thread;
        while (thread->next_thread != nullptr) {
            thread = thread->next_thread;
        }

        thread->next_thread = previous_thread;
    }

    Thread *next_thread = cpu.next_thread;

    cpu.current_thread = next_thread;
    cpu.next_thread = next_thread->next_thread;
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
