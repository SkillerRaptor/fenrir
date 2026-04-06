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
#include "lib/vector.hpp"
#include "memory/pmm.hpp"
#include "scheduler/process.hpp"
#include "scheduler/thread.hpp"
#include "sync/spinlock.hpp"

namespace kernel::scheduler {

extern "C" void switch_process(const Registers *registers);

static i32 s_current_process_id { 0 };
static i32 s_current_thread_id { 0 };

static lib::Vector<Process> s_process_list { };
static Spinlock s_process_list_lock { };

static lib::Vector<Thread> s_thread_list { };
static Spinlock s_thread_list_lock { };

static ProcessId s_kernel_process { -1 };

static void schedule(const Registers &registers);

void initialize()
{
    idt::set_handler(0x20, schedule);

    s_kernel_process = create_process(nullptr);
    logger::debug("Scheduler: Created kernel process with id #%u\n", s_kernel_process);

    logger::info("Scheduler: Initialized\n");
}

__attribute__((noreturn)) void yield()
{
    while (true) {
        cpu::enable_interrupts();
        cpu::halt();
    }
}

ProcessId create_process(vmm::PageMap *page_map)
{
    SpinlockLocker _locker(s_process_list_lock);

    const ProcessId pid = ProcessId { s_current_process_id++ };

    // TODO: Add option to create page map for isolated processes
    // TODO: Copy higher half of page map to always have the kernel mapped

    const Process process {
        .pid = pid,
        .state = Process::State::Idle,
        .page_map = page_map == nullptr ? vmm::get_kernel_page_map() : page_map,
    };

    s_process_list.push_back(process);

    return process.pid;
}

// TODO: Add destroy_process

static void thread_wrapper(void (*entry)(void *), void *user_argument)
{
    entry(user_argument);

    const cpu::Info &current_cpu = cpu::get_local_cpu_info();
    const ThreadId current_thread_id = current_cpu.current_thread;

    SpinlockLocker _thread_list_locker(s_thread_list_lock);
    Thread &current_thread = s_thread_list[current_thread_id.get()];
    current_thread.state = Thread::State::Dead;

    yield();
}

ThreadId create_thread(const ProcessId pid, const u64 cs, void (*entry)(void *), void *user_argument)
{
    SpinlockLocker _locker(s_thread_list_lock);

    if (pid == ProcessId { -1 }) {
        return ThreadId { -1 };
    }

    bool found = false;
    for (usize i = 0; i < s_process_list.size(); ++i) {
        if (s_process_list[i].pid == pid) {
            found = true;
            break;
        }
    }

    if (!found) {
        return ThreadId { -1 };
    }

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
        thread.registers.rsi = reinterpret_cast<u64>(user_argument);
        thread.registers.rdi = reinterpret_cast<u64>(entry),
        thread.registers.rip = reinterpret_cast<u64>(thread_wrapper);
        thread.registers.rsp += boot::get_hhdm_offset();
        thread.registers.ss = thread.registers.cs + 0x08;
    } else {
        thread.registers.rdi = reinterpret_cast<u64>(user_argument),
        thread.registers.rip = reinterpret_cast<u64>(entry);
        thread.registers.ss = thread.registers.cs - 0x08;
    }

    s_thread_list.push_back(thread);

    // TODO: Don't hardcore 4 cpus
    cpu::Info *infos = smp::get_cpu_infos();

    usize least_loaded_cpu = 0;
    usize least_load = 0xffffffffffffffff;
    for (usize i = 0; i < boot::get_mp_response()->cpu_count; ++i) {
        if (infos[i].run_queue.size() < least_load) {
            least_load = infos[i].run_queue.size();
            least_loaded_cpu = i;
        }
    }

    {
        cpu::Info *info = &infos[least_loaded_cpu];
        SpinlockLocker _run_queue_locker(info->run_queue_lock);
        info->run_queue.push_back(thread.tid);
    }

    return thread.tid;
}

ThreadId create_idle_thread()
{
    SpinlockLocker _locker(s_thread_list_lock);

    const ThreadId tid = ThreadId { s_current_thread_id++ };

    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    constexpr u64 cs = 0x28;

    const Thread thread {
        .pid = s_kernel_process,
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

    s_thread_list.push_back(thread);

    return thread.tid;
}

// TODO: Add create thread from current process
// TODO: Add destroy thread

ProcessId get_kernel_process() { return s_kernel_process; }

void schedule(const Registers &registers)
{
    cpu::Info &current_cpu = cpu::get_local_cpu_info();
    const ThreadId current_thread_id = current_cpu.current_thread;

    // TODO: Check if thread is not a ghost and exists
    if (current_thread_id != ThreadId { -1 } && current_thread_id != current_cpu.idle_thread) {
        SpinlockLocker _thread_list_locker(s_thread_list_lock);

        Thread &current_thread = s_thread_list[current_thread_id.get()];
        current_thread.registers = registers;
        if (current_thread.state == Thread::State::Busy) {
            current_thread.state = Thread::State::Idle;

            SpinlockLocker _run_queue_locker(current_cpu.run_queue_lock);
            current_cpu.run_queue.push_back(current_thread.tid);
        }
    }

    ThreadId next_thread_id { -1 };

    {
        SpinlockLocker _run_queue_locker(current_cpu.run_queue_lock);
        if (!current_cpu.run_queue.is_empty()) {
            next_thread_id = current_cpu.run_queue.pop_front();
        }
    }

    // TODO: Add work stealing
    if (next_thread_id == ThreadId { -1 }) {
        next_thread_id = current_cpu.idle_thread;
    }

    current_cpu.current_thread = next_thread_id;

    Registers regs { };
    {
        SpinlockLocker _thread_list_locker(s_thread_list_lock);
        Thread &next_thread = s_thread_list[next_thread_id.get()];
        next_thread.state = Thread::State::Busy;
        regs = next_thread.registers;

        const Thread &current_thread = s_thread_list[current_thread_id.get()];
        if (next_thread.pid != current_thread.pid) {
            SpinlockLocker _process_list_locker(s_process_list_lock);

            const Process *next_process = &s_process_list[next_thread.pid.get()];
            vmm::switch_to_page_map(next_process->page_map);
        }
    }

    apic::send_eoi();

    switch_process(&regs);
}

} // namespace kernel::scheduler
