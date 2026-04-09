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
#include "lib/hash_map.hpp"
#include "lib/vector.hpp"
#include "memory/pmm.hpp"
#include "scheduler/process.hpp"
#include "scheduler/thread.hpp"
#include "sync/spinlock.hpp"

// TODO: Add reaper thread
// It should find every thread in a dead state, free the stack and remove it from the global list
// It should find every process, that has no thread anymore that is alive, free the lower half of the page map and
// remove it from the global list

namespace scheduler {

extern "C" [[noreturn]] void switch_process(const Registers *registers);

static Atomic<i32> s_current_process_id { 0 };
static Atomic<i32> s_current_thread_id { 0 };

static SpinlockProtected<HashMap<ProcessId, Process>> s_process_list { };
static SpinlockProtected<HashMap<ThreadId, Thread>> s_thread_list { };

static ProcessId s_kernel_process_id { -1 };

static void schedule(const Registers &registers);

static Process *get_process(const ProcessId pid)
{
    return s_process_list.with([&](const HashMap<ProcessId, Process> &process_list) {
        Process *process = process_list.get(pid);
        assert(process);
        return process;
    });
}

static Thread *get_thread(const ThreadId tid)
{
    return s_thread_list.with([&](const HashMap<ThreadId, Thread> &thread_list) {
        Thread *thread = thread_list.get(tid);
        assert(thread);
        return thread;
    });
}

void initialize()
{
    idt::set_handler(0x20, schedule);

    s_kernel_process_id = create_process(vmm::get_kernel_page_map());
    logger::debug("Scheduler: Created kernel process with id #%u\n", s_kernel_process_id);

    logger::info("Scheduler: Initialized\n");
}

[[noreturn]] void yield()
{
    while (true) {
        cpu::enable_interrupts();
        cpu::halt();
    }
}

ProcessId create_process(vmm::PageMap *page_map)
{
    assert(page_map);

    const ProcessId pid = ProcessId { s_current_process_id.fetch_add(1) };

    // TODO: Copy higher half of page map to always have the kernel mapped

    const Process process {
        .pid = pid,
        .state = Process::State::Idle,
        .page_map = page_map,
    };

    s_process_list.with([&](HashMap<ProcessId, Process> &process_list) { process_list.insert(pid, process); });

    return process.pid;
}

static void thread_wrapper(void (*entry)())
{
    entry();

    // TODO: Find a better way to get the current thread id
    // NOTE: Maybe pass the thread id as argument or make the thread blocking at the time
    // NOTE: The current thread id could change mid way and give the wrong result, resulting in a race condition

    const cpu::Info &current_cpu = cpu::get_local_cpu_info();
    Thread *current_thread = get_thread(current_cpu.current_tid);
    current_thread->state = Thread::State::Dead;

    yield();
}

ThreadId create_thread(const ProcessId pid, const u64 cs, void (*entry)())
{
    assert(pid != ProcessId { -1 });
    assert(entry);

    s_process_list.with([&](const HashMap<ProcessId, Process> &process_list) { assert(process_list.contains(pid)); });

    const ThreadId tid = ThreadId { s_current_thread_id.fetch_add(1) };

    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    Thread thread {
        .pid = pid,
        .tid = tid,
        .state = Thread::State::Idle,
        .registers = {
            .cs = cs,
            .flags = 1 << 9 | 1 << 1,
            .rsp = stack,
        },
        .stack = reinterpret_cast<u8*>(stack),
        .stack_size = memory::s_page_size,
    };

    if (cs == 0x28) {
        thread.registers.rdi = reinterpret_cast<u64>(entry),
        thread.registers.rip = reinterpret_cast<u64>(thread_wrapper);
        thread.registers.rsp += boot::get_hhdm_offset();
        thread.registers.ss = thread.registers.cs + 0x08;
    } else {
        thread.registers.rip = reinterpret_cast<u64>(entry);
        thread.registers.ss = thread.registers.cs - 0x08;
    }

    // NOTE: Map stack
    s_process_list.with([&](const HashMap<ProcessId, Process> &process_list) {
        const Process *process = process_list.get(thread.pid);
        assert(process);

        vmm::map(
            process->page_map,
            stack - memory::s_page_size,
            thread.registers.rsp - memory::s_page_size,
            vmm::Attribute::Write | (cs == 0x28 ? vmm::Attribute::None : vmm::Attribute::User));
    });

    s_thread_list.with([&](HashMap<ThreadId, Thread> &thread_list) { thread_list.insert(thread.tid, thread); });

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

    infos[least_loaded_cpu].run_queue.push_back(thread.tid);

    return thread.tid;
}

ThreadId create_idle_thread()
{
    const ThreadId tid = ThreadId { s_current_thread_id.fetch_add(1) };

    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    constexpr u64 cs = 0x28;

    const Thread thread {
        .pid = s_kernel_process_id,
        .tid = tid,
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
    };

    s_thread_list.with([&](HashMap<ThreadId, Thread> &thread_list) { thread_list.insert(thread.tid, thread); });

    return thread.tid;
}

void destroy_thread(ThreadId) { }

// TODO: Add create thread from current process

ProcessId get_kernel_process() { return s_kernel_process_id; }

void schedule(const Registers &registers)
{
    cpu::Info &cpu = cpu::get_local_cpu_info();

    if (cpu.run_queue.is_empty()) {
        if (cpu.current_tid != ThreadId { -1 }) {
            apic::send_eoi();
            switch_process(&registers);
        }

        const Thread *idle_thread = get_thread(cpu.idle_tid);

        apic::send_eoi();
        switch_process(&idle_thread->registers);
    }

    if (cpu.current_tid != ThreadId { -1 }) {
        Thread *current_thread = get_thread(cpu.current_tid);

        // FIXME: Make this work with reaper thread
        if (current_thread->state == Thread::State::Dead) {
            return;
        }

        current_thread->registers = registers;
        current_thread->state = Thread::State::Idle;

        cpu.run_queue.push_back(current_thread->tid);
    }

    const ThreadId next_tid = cpu.run_queue.pop_front();

    Thread *next_thread = get_thread(next_tid);

    const vmm::PageMap *page_map { nullptr };
    if (cpu.current_tid != ThreadId { -1 }) {
        const Thread *current_thread = get_thread(cpu.current_tid);
        if (current_thread->pid != next_thread->pid) {
            const Process *next_process = get_process(next_thread->pid);
            page_map = next_process->page_map;
        }
    }

    cpu.current_tid = next_tid;
    next_thread->state = Thread::State::Busy;

    if (page_map) {
        vmm::switch_to_page_map(page_map);
    }

    apic::send_eoi();
    switch_process(&next_thread->registers);
}

} // namespace scheduler
