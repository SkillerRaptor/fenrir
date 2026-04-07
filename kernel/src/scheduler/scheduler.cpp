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
#include "lib/hash_map.hpp"
#include "lib/vector.hpp"
#include "memory/pmm.hpp"
#include "scheduler/process.hpp"
#include "scheduler/thread.hpp"
#include "sync/spinlock.hpp"

namespace scheduler {

extern "C" void switch_process(const Registers *registers);

static i32 s_current_process_id { 0 };
static i32 s_current_thread_id { 0 };

static SpinlockProtected<HashMap<ProcessId, Process>> s_process_list {};
static SpinlockProtected<HashMap<ThreadId, Thread>> s_thread_list {};

static ProcessId s_kernel_process_id { -1 };

static void schedule(const Registers &registers);

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

    const ProcessId pid = ProcessId { s_current_process_id++ };

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

    const cpu::Info &current_cpu = cpu::get_local_cpu_info();
    const ThreadId current_thread_id = current_cpu.current_thread;

    s_thread_list.with([&](const HashMap<ThreadId, Thread> &thread_list) {
        Thread *current_thread = thread_list.get(current_thread_id);
        assert(current_thread);

        current_thread->state = Thread::State::Dead;
    });

    yield();
}

ThreadId create_thread(const ProcessId pid, const u64 cs, void (*entry)())
{
    assert(pid != ProcessId { -1 });
    assert(entry);

    s_process_list.with([&](const HashMap<ProcessId, Process> &process_list) { assert(process_list.contains(pid)); });

    const ThreadId tid = ThreadId { s_current_thread_id++ };

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
        const usize load = infos[i].run_queue.with([&](Queue<ThreadId> &run_queue) { return run_queue.size(); });

        if (load < least_load) {
            least_load = load;
            least_loaded_cpu = i;
        }
    }

    infos[least_loaded_cpu].run_queue.with([&](Queue<ThreadId> &run_queue) { run_queue.push_back(thread.tid); });

    return thread.tid;
}

ThreadId create_idle_thread()
{
    const ThreadId tid = ThreadId { s_current_thread_id++ };

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
    cpu::Info &current_cpu = cpu::get_local_cpu_info();
    const ThreadId current_thread_id = current_cpu.current_thread;

    // TODO: Check if thread is not a ghost and exists
    if (current_thread_id != ThreadId { -1 } && current_thread_id != current_cpu.idle_thread) {
        const bool is_busy = s_thread_list.with([&](const HashMap<ThreadId, Thread> &thread_list) {
            Thread *current_thread = thread_list.get(current_thread_id);
            assert(current_thread);

            current_thread->registers = registers;

            if (current_thread->state != Thread::State::Busy) {
                return false;
            }

            current_thread->state = Thread::State::Idle;
            return true;
        });

        if (is_busy) {
            current_cpu.run_queue.with([&](Queue<ThreadId> &run_queue) { run_queue.push_back(current_thread_id); });
        }
    }

    const ThreadId next_thread_id = current_cpu.run_queue.with([&](Queue<ThreadId> &run_queue) {
        if (run_queue.is_empty()) {
            return current_cpu.idle_thread;
        }

        return run_queue.pop_front();
    });

    // TODO: Add work stealing

    current_cpu.current_thread = next_thread_id;

    const Registers regs = s_thread_list.with([&](const HashMap<ThreadId, Thread> &thread_list) {
        Thread *next_thread = thread_list.get(next_thread_id);
        assert(next_thread);

        next_thread->state = Thread::State::Busy;

        const Thread *current_thread = thread_list.get(current_thread_id);
        if (current_thread && next_thread->pid != current_thread->pid) {
            s_process_list.with([&](const HashMap<ProcessId, Process> &process_list) {
                const Process *next_process = process_list.get(next_thread->pid);
                assert(next_process);

                vmm::switch_to_page_map(next_process->page_map);
            });
        }

        return next_thread->registers;
    });

    apic::send_eoi();

    switch_process(&regs);
}

} // namespace scheduler
