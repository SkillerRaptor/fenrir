/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/idt.h"

#include <stddef.h>
#include <stdint.h>

#include "arch/x86_64/cpu.h"
#include "arch/x86_64/registers.h"
#include "lib/log.h"

enum Attribute : uint8_t {
    ATTRIBUTE_TRAP_GATE = 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0,
    ATTRIBUTE_INTERRUPT_GATE = 1 << 3 | 1 << 2 | 1 << 1 | 0 << 0,
    ATTRIBUTE_KERNEL_PRIVILEGE = 0 << 6 | 0 << 5,
    ATTRIBUTE_PRESENT = 1 << 7,
};

struct [[gnu::packed]] Entry {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
};

static_assert(sizeof(struct Entry) == 16);

struct [[gnu::packed]] Descriptor {
    uint16_t size;
    uint64_t address;
};

static_assert(sizeof(struct Descriptor) == 10);

extern void *interrupt_handlers[];
extern void load_idt(struct Descriptor *descriptor);

static const char *const EXCEPTIONS[31] = {
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

static struct Entry s_entries[256] = { };
static InterruptHandler s_interrupt_handlers[256] = { };

static struct Entry create_entry(void *handler, enum Attribute attributes)
{
    uintptr_t address = (uintptr_t) handler;

    return (struct Entry) {
        .offset_low = (uint16_t) address,
        .segment = 0x28,
        .ist = 0,
        .attributes = attributes,
        .offset_middle = (uint16_t) (address >> 16),
        .offset_high = (uint32_t) (address >> 32),
        .reserved = 0,
    };
}

static void handle_exception(struct Registers *registers)
{
    LOG_ERROR("\n");
    LOG_ERROR("%s occured!\n", EXCEPTIONS[registers->isr]);
    LOG_ERROR("\n");

    LOG_ERROR("Registers:\n");
    LOG_ERROR(
        "  rax=%#018lx rbx=%#018lx rcx=%#018lx rdx=%#018lx\n",
        registers->rax,
        registers->rbx,
        registers->rcx,
        registers->rdx);
    LOG_ERROR(
        "  rsi=%#018lx rdi=%#018lx rbp=%#018lx rsp=%#018lx\n",
        registers->rsi,
        registers->rdi,
        registers->rbp,
        registers->rsp);
    LOG_ERROR(
        "   r8=%#018lx  r9=%#018lx r10=%#018lx r11=%#018lx\n",
        registers->r8,
        registers->r9,
        registers->r10,
        registers->r11);
    LOG_ERROR(
        "  r12=%#018lx r13=%#018lx r14=%#018lx r15=%#018lx\n",
        registers->r12,
        registers->r13,
        registers->r14,
        registers->r15);
    LOG_ERROR("  rip=%#018lx rfl=%#018lx\n", registers->rip, registers->flags);
    LOG_ERROR("\n");

    LOG_ERROR("Segments:\n");
    LOG_ERROR("  cs=%#04lx\n", registers->cs);
    LOG_ERROR("  ss=%#04lx\n", registers->ss);
    LOG_ERROR("\n");

    LOG_ERROR("Halting System...\n");
    LOG_ERROR("\n");

    cpu_hcf();
}

void idt_init()
{
    for (size_t i = 0; i < 256; ++i) {
        s_entries[i] = create_entry(
            interrupt_handlers[i],
            ATTRIBUTE_PRESENT | ATTRIBUTE_KERNEL_PRIVILEGE | ATTRIBUTE_INTERRUPT_GATE);
    }

    s_interrupt_handlers[0] = handle_exception;
    s_interrupt_handlers[1] = handle_exception;
    s_interrupt_handlers[2] = handle_exception;
    s_interrupt_handlers[3] = handle_exception;
    s_interrupt_handlers[4] = handle_exception;
    s_interrupt_handlers[5] = handle_exception;
    s_interrupt_handlers[6] = handle_exception;
    s_interrupt_handlers[7] = handle_exception;
    s_interrupt_handlers[8] = handle_exception;

    s_interrupt_handlers[10] = handle_exception;
    s_interrupt_handlers[11] = handle_exception;
    s_interrupt_handlers[12] = handle_exception;
    s_interrupt_handlers[13] = handle_exception;
    s_interrupt_handlers[14] = handle_exception;

    s_interrupt_handlers[16] = handle_exception;
    s_interrupt_handlers[17] = handle_exception;
    s_interrupt_handlers[18] = handle_exception;
    s_interrupt_handlers[19] = handle_exception;
    s_interrupt_handlers[20] = handle_exception;

    s_interrupt_handlers[30] = handle_exception;

    struct Descriptor descriptor = {
        .size = sizeof(s_entries) - 1,
        .address = (uintptr_t) s_entries,
    };

    load_idt(&descriptor);

    LOG_INFO("IDT: Initialized\n");
}

void idt_register_handler(uint8_t isr, InterruptHandler interrupt_handler)
{
    s_interrupt_handlers[isr] = interrupt_handler;
}

[[gnu::used]] void interrupt_raise(struct Registers *registers)
{
    InterruptHandler interrupt_handler = s_interrupt_handlers[registers->isr];
    if (interrupt_handler) {
        interrupt_handler(registers);
    }

    // TODO: Implement EOI
    cpu_hcf();
}
