/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/idt.hpp"

#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/registers.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/types.hpp"

namespace kernel::idt {

#define ATTRIBUTE_PRESENT (1 << 7)
#define ATTRIBUTE_INTERRUPT_GATE (1 << 1 | 1 << 2 | 1 << 3)

struct Entry {
    u16 offset_low { 0 };
    u16 selector { 0 };
    u8 ist { 0 };
    u8 attributes { 0 };
    u16 offset_mid { 0 };
    u32 offset_high { 0 };
    u32 reserved { 0 };
} __attribute__((packed));

struct Descriptor {
    u16 size { 0 };
    u64 address { 0 };
} __attribute__((packed));

extern "C" void load_idt(const Descriptor *descriptor);

static Entry s_entries[256] { };
static Descriptor s_descriptor { };

static Entry create_entry(void *handler, const u8 attributes)
{
    const auto address = reinterpret_cast<u64>(handler);

    return {
        .offset_low = static_cast<u16>(address & 0xffff),
        .selector = 0x28,
        .ist = 0,
        .attributes = attributes,
        .offset_mid = static_cast<u16>((address >> 16) & 0xffff),
        .offset_high = static_cast<u32>((address >> 32) & 0xffffffff),
        .reserved = 0,
    };
}

extern "C" void *interrupt_handlers[];

void initialize()
{
    for (usize i = 0; i < 256; ++i) {
        s_entries[i] = create_entry(interrupt_handlers[i], ATTRIBUTE_PRESENT | ATTRIBUTE_INTERRUPT_GATE);
    }

    s_descriptor.size = sizeof(s_entries) - 1;
    s_descriptor.address = reinterpret_cast<u64>(&s_entries[0]);

    load_idt(&s_descriptor);

    logger::ok("IDT: Initialized\n");
}

__attribute__((noreturn)) extern "C" void interrupt_raise(const Registers *)
{
    while (true) {
        cpu::disable_interrupts();
        cpu::halt();
    }
}

} // namespace kernel::idt
