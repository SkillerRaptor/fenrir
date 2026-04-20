/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/panic.hpp"

#include "acpi/apic.hpp"
#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "scheduler/process.hpp"
#include "scheduler/scheduler.hpp"

static void print_registers(const Registers &registers)
{
    logger::fatal("Registers:\n");
    logger::fatal(
        "  rax=0x%016lx rbx=0x%016lx rcx=0x%016lx rdx=0x%016lx\n",
        registers.rax,
        registers.rbx,
        registers.rcx,
        registers.rdx);
    logger::fatal(
        "  rsi=0x%016lx rdi=0x%016lx rbp=0x%016lx rsp=0x%016lx\n",
        registers.rsi,
        registers.rdi,
        registers.rbp,
        registers.rsp);
    logger::fatal(
        "   r8=0x%016lx  r9=0x%016lx r10=0x%016lx r11=0x%016lx\n",
        registers.r8,
        registers.r9,
        registers.r10,
        registers.r11);
    logger::fatal(
        "  r12=0x%016lx r13=0x%016lx r14=0x%016lx r15=0x%016lx\n",
        registers.r12,
        registers.r13,
        registers.r14,
        registers.r15);
    logger::fatal(
        "  rip=0x%016lx  cs=0x%02lx  ss=0x%02lx  rflags=0x%016lx\n",
        registers.rip,
        registers.cs,
        registers.ss,
        registers.flags);
}

static void print_control_registers()
{
    volatile u64 cr0 = 0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    volatile u64 cr2 = 0;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));

    volatile u64 cr3 = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));

    volatile u64 cr4 = 0;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));

    logger::fatal("Control Registers:\n");
    logger::fatal("  cr0=0x%016lx  cr2=0x%016lx\n", cr0, cr2);
    logger::fatal("  cr3=0x%016lx  cr4=0x%016lx\n", cr3, cr4);
}

[[noreturn]] void __panic(const char *file, const u32 line, const char *function, const char *message)
{
    cpu::disable_interrupts();
    apic::send_ipi(0xff, 0xfe);

    const cpu::Core &core = cpu::current();

    logger::fatal("\n");
    logger::fatal("Panic in \033[38;2;255;215;0m%s\033[0m at \033[38;2;0;128;0m%s:%u\n", function, file, line);
    if (message) {
        logger::fatal("  -> %s\n", message);
    }
    logger::fatal("\n");

    logger::fatal("Current State:\n");
    logger::fatal("  Core: #%u\n", core.id);
    if (core.current_thread) {
        logger::fatal("  Process: #%d \n", core.current_thread->process->id.get());
        logger::fatal("  Thread: #%d\n", core.current_thread->id.get());
    } else {
        logger::fatal("  Process: <unknown>\n");
        logger::fatal("  Thread: <unknown>\n");
    }
    logger::fatal("\n");

    stacktrace::print(50);
    logger::fatal("\n");
    logger::fatal("Halting System...\n");

    cpu::hcf();
}

static const char *s_exceptions[] = {
    "Divide-by-zero Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "<invalid>",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment-Fault",
    "General-Protection-Fault",
    "Page Fault",
    "<invalid>",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "<invalid>",
    "Security Exception",
};

void __panic_exception(const Registers &registers)
{
    if (registers.cs == 0x28) {
        apic::send_ipi(0xff, 0xfe);
    }

    logger::fatal("\n");
    logger::fatal("%s occurred!\n", s_exceptions[registers.isr]);

    switch (registers.isr) {
    case 0x0e: {
        volatile u64 cr2 = 0;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));

        logger::fatal("  at 0x%016lx\n", cr2);

        const u64 error = registers.error;
        logger::fatal("  because");
        logger::log(" %s,", (error & (1 << 0)) ? "protection violation" : "non-present page");
        logger::log(" %s,", (error & (1 << 1)) ? "write access" : "read access");
        logger::log(" %s", (error & (1 << 2)) ? "user-mode" : "kernel-mode");

        if (error & (1 << 3)) {
            logger::log(", reserved bit set in PTE");
        }

        if (error & (1 << 4)) {
            logger::log(", instruction fetch (NX fault)");
        }

        logger::log("\n");

        break;
    }
    case 0x0a:
    case 0x0b:
    case 0x0c:
    case 0x0d: {
        if (registers.error == 0) {
            break;
        }

        const u8 table = (registers.error >> 1) & 0b11;
        const u16 index = (registers.error >> 3) & 0x1fff;
        const char *table_name = [table]() {
            switch (table) {
            case 0:
                return "GDT";
            case 1:
                return "IDT";
            case 2:
                return "LDT";
            case 3:
                return "IDT";
            default:
                __builtin_unreachable();
            }
        }();

        logger::fatal("  in %s at at %u\n", table_name, index);

        break;
    }
    default:
        break;
    }
    logger::fatal("\n");

    print_registers(registers);
    logger::fatal("\n");

    const cpu::Core &core = cpu::current();
    if (registers.cs == 0x28) {
        print_control_registers();
        logger::fatal("\n");

        logger::fatal("Current State:\n");
        logger::fatal("  Core: #%u\n", core.id);
        if (core.current_thread) {
            logger::fatal("  Process: #%d \n", core.current_thread->process->id.get());
            logger::fatal("  Thread: #%d\n", core.current_thread->id.get());
        } else {
            logger::fatal("  Process: <unknown>\n");
            logger::fatal("  Thread: <unknown>\n");
        }
        logger::fatal("\n");

        stacktrace::print(50, registers.rip);

        logger::fatal("\n");
        logger::fatal("Halting System...\n");

        cpu::hcf();
    } else {
        struct StackFrame {
            u64 rbp = 0;
            u64 rip = 0;
        };

        logger::fatal("Stacktrace:\n");

        u64 rbp = registers.rbp;
        u64 rip = registers.rip;
        for (usize i = 0; rbp != 0 && i < 50; ++i) {
            // TODO: Resolve user symbols per ELF symbol table
            logger::fatal("  %02lu. \033[38;2;0;0;255m0x%016lx \033[0min \033[38;2;255;215;0m??\n", i + 1, rip);

            const u64 physical_address = vmm::virtual_to_physical(core.current_thread->process->page_map, rbp);
            if (physical_address == 0) {
                break;
            }

            const StackFrame *frame = reinterpret_cast<const StackFrame *>(physical_address + boot::get_hhdm_offset());
            rip = frame->rip;
            rbp = frame->rbp;
        }
        logger::fatal("\n");

        cpu::enter_critical();
        core.current_thread->state = Thread::State::Dead;
        cpu::leave_critical();

        logger::fatal("Current State:\n");
        logger::fatal("  Core: #%u\n", core.id);
        if (core.current_thread) {
            logger::fatal("  Process: #%d \n", core.current_thread->process->id.get());
            logger::fatal("  Thread: #%d\n", core.current_thread->id.get());
        } else {
            logger::fatal("  Process: <unknown>\n");
            logger::fatal("  Thread: <unknown>\n");
        }

        logger::fatal("\n");
        logger::fatal("Terminating process...\n");

        scheduler::yield();
    }
}
