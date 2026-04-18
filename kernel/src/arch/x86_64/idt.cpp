/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/idt.hpp"

#include "acpi/apic.hpp"
#include "arch/x86_64/cpu.hpp"
#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "lib/assert.hpp"
#include "lib/bitflags.hpp"

namespace idt {

enum class Attribute : u8 {
    TrapGate = 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0,
    InterruptGate = 1 << 3 | 1 << 2 | 1 << 1 | 0 << 0,
    KernelPrivilege = 0 << 6 | 0 << 5,
    UserPrivilege = 1 << 6 | 1 << 5,
    Present = 1 << 7
};

DECLARE_BITFLAG(Attribute);

struct Entry {
    u16 offset_low = 0;
    u16 selector = 0;
    u8 ist = 0;
    u8 attributes = 0;
    u16 offset_mid = 0;
    u32 offset_high = 0;
    u32 reserved = 0;
} __attribute__((packed));

struct Descriptor {
    u16 size = 0;
    u64 address = 0;
} __attribute__((packed));

extern "C" void load_idt(const Descriptor *descriptor);

extern "C" void *interrupt_handlers[];

static Entry s_entries[256] { };
static Descriptor s_descriptor { };
static InterruptHandler s_interrupt_handlers[256] { };

static Entry create_entry(void *handler, const Attribute attributes)
{
    const u64 address = reinterpret_cast<u64>(handler);

    return {
        .offset_low = static_cast<u16>(address & 0xffff),
        .selector = 0x28,
        .ist = 0,
        .attributes = static_cast<u8>(attributes),
        .offset_mid = static_cast<u16>((address >> 16) & 0xffff),
        .offset_high = static_cast<u32>((address >> 32) & 0xffffffff),
        .reserved = 0,
    };
}

#define ENUMERATE_EXCEPTIONS                                                                 \
    _ENUMERATE_EXCEPTION(0, divide_by_zero, "Divide-by-zero Error")                          \
    _ENUMERATE_EXCEPTION(1, debug, "Debug")                                                  \
    _ENUMERATE_EXCEPTION(2, non_maskable_interrupt, "Non-maskable Interrupt")                \
    _ENUMERATE_EXCEPTION(3, breakpoint, "Breakpoint")                                        \
    _ENUMERATE_EXCEPTION(4, overflow, "Overflow")                                            \
    _ENUMERATE_EXCEPTION(5, bound_range_exceeded, "Bound Range Exceeded")                    \
    _ENUMERATE_EXCEPTION(6, invalid_opcode, "Invalid Opcode")                                \
    _ENUMERATE_EXCEPTION(7, device_not_available, "Device Not Available")                    \
    _ENUMERATE_EXCEPTION(8, double_fault, "Double Fault")                                    \
    _ENUMERATE_EXCEPTION(10, invalid_tss, "Invalid TSS")                                     \
    _ENUMERATE_EXCEPTION(11, segment_not_present, "Segment Not Present")                     \
    _ENUMERATE_EXCEPTION(12, stack_segment_fault, "Stack-Segment-Fault")                     \
    _ENUMERATE_EXCEPTION(13, general_protection_fault, "General-Protection-Fault")           \
    _ENUMERATE_EXCEPTION(16, x87_floating_point_exception, "x87 Floating-Point Exception")   \
    _ENUMERATE_EXCEPTION(17, alignment_check, "Alignment Check")                             \
    _ENUMERATE_EXCEPTION(18, machine_check, "Machine Check")                                 \
    _ENUMERATE_EXCEPTION(19, simd_floating_point_exception, "SIMD Floating-Point Exception") \
    _ENUMERATE_EXCEPTION(20, virtualization_exception, "Virtualization Exception")           \
    _ENUMERATE_EXCEPTION(30, security_exception, "Security Exception")

#define _ENUMERATE_EXCEPTION(i, fn, exception)                                   \
    __attribute__((noreturn)) void fn(const Registers &registers)                \
    {                                                                            \
        apic::send_ipi(0xff, 0xfe);                                              \
                                                                                 \
        logger::err(exception " occured with error code %u\n", registers.error); \
        logger::err("Register dump:\n");                                         \
        logger::err(                                                             \
            "  rax=0x%016x rbx=0x%016x rcx=0x%016x rdx=0x%016x\n",               \
            registers.rax,                                                       \
            registers.rbx,                                                       \
            registers.rcx,                                                       \
            registers.rdx);                                                      \
        logger::err(                                                             \
            "  rsi=0x%016x rdi=0x%016x rbp=0x%016x rsp=0x%016x\n",               \
            registers.rsi,                                                       \
            registers.rdi,                                                       \
            registers.rbp,                                                       \
            registers.rsp);                                                      \
        logger::err(                                                             \
            "   r8=0x%016x  r9=0x%016x r10=0x%016x r11=0x%016x\n",               \
            registers.r8,                                                        \
            registers.r9,                                                        \
            registers.r10,                                                       \
            registers.r11);                                                      \
        logger::err(                                                             \
            "  r12=0x%016x r13=0x%016x r14=0x%016x r15=0x%016x\n",               \
            registers.r12,                                                       \
            registers.r13,                                                       \
            registers.r14,                                                       \
            registers.r15);                                                      \
        logger::err(                                                             \
            "  rip=0x%016x  cs=0x%016x  ss=0x%016x flg=0x%016x\n",               \
            registers.rip,                                                       \
            registers.cs,                                                        \
            registers.ss,                                                        \
            registers.flags);                                                    \
                                                                                 \
        stacktrace::print(10);                                                   \
                                                                                 \
        cpu::hcf();                                                              \
    }

ENUMERATE_EXCEPTIONS

#undef _ENUMERATE_EXCEPTION

__attribute__((noreturn)) void page_fault(const Registers &registers)
{
    apic::send_ipi(0xff, 0xfe);

    volatile u64 faulting_address = 0;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    const cpu::Core &core = cpu::current();
    logger::err(
        "Page Fault at address 0x%016llx on CPU #%u and Thread #%d\n",
        faulting_address,
        core.id,
        core.current_thread->id.get());

    if (registers.error & 0b00001) {
        logger::err(" - Page-level protection violation\n");
    } else {
        logger::err(" - Non-present page\n");
    }

    if (registers.error & 0b00010) {
        logger::err(" - Write access\n");
    } else {
        logger::err(" - Read access\n");
    }

    if (registers.error & 0b00100) {
        logger::err(" - User-mode\n");
    } else {
        logger::err(" - Kernel-mode\n");
    }

    if (registers.error & 0b01000) {
        logger::err(" - Reserved bit set\n");
    }

    if (registers.error & 0b10000) {
        logger::err(" - Instruction fetch fault\n");
    }

    logger::err("Register dump:\n");
    logger::err(
        "  rax=0x%016x rbx=0x%016x rcx=0x%016x rdx=0x%016x\n",
        registers.rax,
        registers.rbx,
        registers.rcx,
        registers.rdx);
    logger::err(
        "  rsi=0x%016x rdi=0x%016x rbp=0x%016x rsp=0x%016x\n",
        registers.rsi,
        registers.rdi,
        registers.rbp,
        registers.rsp);
    logger::err(
        "   r8=0x%016x  r9=0x%016x r10=0x%016x r11=0x%016x\n",
        registers.r8,
        registers.r9,
        registers.r10,
        registers.r11);
    logger::err(
        "  r12=0x%016x r13=0x%016x r14=0x%016x r15=0x%016x\n",
        registers.r12,
        registers.r13,
        registers.r14,
        registers.r15);
    logger::err(
        "  rip=0x%016x  cs=0x%016x  ss=0x%016x flg=0x%016x\n",
        registers.rip,
        registers.cs,
        registers.ss,
        registers.flags);

    stacktrace::print(10);

    cpu::hcf();
}

void initialize()
{
    for (usize i = 0; i < 256; ++i) {
        s_entries[i] = create_entry(
            interrupt_handlers[i],
            Attribute::KernelPrivilege | Attribute::Present | Attribute::InterruptGate);
    }

#define _ENUMERATE_EXCEPTION(i, fn, err) s_interrupt_handlers[i] = fn;
    ENUMERATE_EXCEPTIONS
#undef _ENUMERATE_EXCEPTION

    s_interrupt_handlers[14] = page_fault;

    s_descriptor.size = sizeof(s_entries) - 1;
    s_descriptor.address = reinterpret_cast<u64>(s_entries);

    load();

    logger::info("IDT: Initialized\n");
}

void load() { load_idt(&s_descriptor); }

void set_handler(const u8 isr, const InterruptHandler handler)
{
    assert(handler);
    s_interrupt_handlers[isr] = handler;
}

extern "C" void interrupt_raise(const Registers *registers)
{
    if (s_interrupt_handlers[registers->isr]) {
        s_interrupt_handlers[registers->isr](*registers);
    } else {
        cpu::hcf();
    }

    apic::send_eoi();
}

} // namespace idt
