/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/scheduler/scheduler.hpp"

#include "kernel/acpi/apic.hpp"
#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/idt.hpp"
#include "kernel/arch/x86_64/registers.hpp"
#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/memory.hpp"
#include "kernel/lib/vector.hpp"
#include "kernel/memory/pmm.hpp"
#include "kernel/sync/spinlock.hpp"

namespace kernel::scheduler {

extern "C" void switch_process(const Registers *registers);

static Process::Id s_current_process_id = 0;
static Thread::Id s_current_thread_id = 0;

static Vector<Process> s_process_list { };
static Spinlock s_process_list_lock { };

static Vector<Thread> s_thread_list { };
static Spinlock s_thread_list_lock { };

static Process::Id s_kernel_process = -1;

static void schedule(const Registers &registers);

void initialize()
{
    idt::set_handler(0x20, schedule);

    s_kernel_process = create_process();
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

Process::Id create_process()
{
    SpinlockLocker _locker(s_process_list_lock);

    const Process::Id pid = s_current_process_id;
    ++s_current_process_id;

    // TODO: Add option to create page map for isolated processes
    vmm::PageMap *page_map = vmm::get_kernel_page_map();

    const Process process {
        .pid = pid,
        .state = Process::State::Idle,
        .page_map = page_map,
    };

    s_process_list.push_back(process);

    logger::debug("Scheduler: Created process #%u\n", process.pid);

    return process.pid;
}

// TODO: Add destroy_process

Thread::Id create_thread(const Process::Id pid, void (*function)(void *), void *user_argument)
{
    SpinlockLocker _locker(s_thread_list_lock);

    if (pid == -1) {
        return -1;
    }

    bool found = false;
    for (usize i = 0; i < s_process_list.size(); ++i) {
        if (s_process_list[i].pid == pid) {
            found = true;
            break;
        }
    }

    if (!found) {
        return -1;
    }

    const Thread::Id tid = s_current_thread_id;
    ++s_current_thread_id;

    const u64 stack = reinterpret_cast<u64>(pmm::allocate(1, true)) + memory::s_page_size;

    constexpr u64 cs = 0x28;

    const Thread thread {
        .pid = pid,
        .tid = tid,
        .state = Thread::State::Idle,
        .registers = {
            .rdi = reinterpret_cast<u64>(user_argument),
            .rbp = 0,
            // NOTE: Iret Frame
            .rip = reinterpret_cast<u64>(function),
            .cs = cs,
            .flags = 1 << 9 | 1 << 1,
            .rsp = stack + boot::get_hhdm_offset(),
            .ss = cs + 0x08,
        },
        .stack = reinterpret_cast<u8*>(stack),
        .stack_size = memory::s_page_size,
    };

    s_thread_list.push_back(thread);

    // TODO: Don't hardcore 4 cpus
    cpu::Info *infos = smp::get_cpu_infos();

    usize least_loaded_cpu = 0;
    usize least_load = 0xffffffffffffffff;
    for (usize i = 0; i < 4; ++i) {
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

    logger::debug("Scheduler: Created thread #%u for process #%u\n", thread.tid, thread.pid);

    return thread.tid;
}

Thread::Id create_idle_thread()
{
    SpinlockLocker _locker(s_thread_list_lock);

    const Thread::Id tid = s_current_thread_id;
    ++s_current_thread_id;

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

    logger::debug("Scheduler: Created idle thread #%u for process #%u\n", thread.tid, thread.pid);

    return thread.tid;
}

// TODO: Add create thread from current process
// TODO: Add destroy thread

Process::Id get_kernel_process() { return s_kernel_process; }

void schedule(const Registers &registers)
{
    cpu::Info &current_cpu = cpu::get_local_cpu_info();
    const Thread::Id current_thread_id = current_cpu.current_thread;

    // TODO: Check if thread is not a ghost and exists
    if (current_thread_id != -1 && current_thread_id != current_cpu.idle_thread) {
        SpinlockLocker _thread_list_locker(s_thread_list_lock);

        Thread &current_thread = s_thread_list[current_thread_id];
        current_thread.registers = registers;
        if (current_thread.state == Thread::State::Busy) {
            current_thread.state = Thread::State::Idle;

            SpinlockLocker _run_queue_locker(current_cpu.run_queue_lock);
            current_cpu.run_queue.push_back(current_thread.tid);
        }
    }

    Thread::Id next_thread_id = -1;

    {
        SpinlockLocker _run_queue_locker(current_cpu.run_queue_lock);
        if (!current_cpu.run_queue.is_empty()) {
            next_thread_id = current_cpu.run_queue.pop_front();
        }
    }

    // TODO: Add work stealing
    if (next_thread_id == -1) {
        next_thread_id = current_cpu.idle_thread;
    }

    current_cpu.current_thread = next_thread_id;

    Registers regs { };
    {
        SpinlockLocker _thread_list_locker(s_thread_list_lock);
        const Thread &current_thread = s_thread_list[current_thread_id];
        Thread &next_thread = s_thread_list[next_thread_id];
        next_thread.state = Thread::State::Busy;
        regs = next_thread.registers;

        if (next_thread.pid != current_thread.pid) {
            SpinlockLocker _process_list_locker(s_process_list_lock);

            const Process *next_process = &s_process_list[next_thread.pid];
            vmm::switch_to_page_map(next_process->page_map);
        }
    }

    apic::send_eoi();

    switch_process(&regs);
}

} // namespace kernel::scheduler
