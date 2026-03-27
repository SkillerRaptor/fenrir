/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/gdt.hpp"

#include "kernel/core/logger.hpp"
#include "kernel/core/types.hpp"

namespace kernel::gdt {

#define ACCESS_ATTRIBUTE_NONE (0 << 0)
#define ACCESS_ATTRIBUTE_ACCESS (1 << 0)
#define ACCESS_ATTRIBUTE_READ_WRITE (1 << 1)
#define ACCESS_ATTRIBUTE_EXECUTABLE (1 << 3)
#define ACCESS_ATTRIBUTE_CODE_DATA (1 << 4)
#define ACCESS_ATTRIBUTE_PRESENT (1 << 7)

#define FLAG_ATTRIBUTE_NONE (0 << 0)
#define FLAG_ATTRIBUTE_64 (1 << 1)
#define FLAG_ATTRIBUTE_32 (1 << 2)
#define FLAG_ATTRIBUTE_4K (1 << 3)

struct Entry {
    u16 limit_low { 0 };
    u16 base_low { 0 };
    u8 base_middle { 0 };
    u8 access { 0 };
    u8 limit_high : 4 { 0 };
    u8 flags : 4 { 0 };
    u8 base_high { 0 };
} __attribute__((packed));

struct Descriptor {
    u16 size { 0 };
    u64 address { 0 };
} __attribute__((packed));

extern "C" void load_gdt(const Descriptor *descriptor);
extern "C" void reload_segments();

static Entry s_entries[7] { };
static Descriptor s_descriptor { };

static Entry create_entry(const u32 base, const u32 limit, const u8 access, const u8 flags)
{
    return {
        .limit_low = static_cast<u16>(limit & 0xffff),
        .base_low = static_cast<u16>(base & 0xffff),
        .base_middle = static_cast<u8>((base >> 16) & 0xff),
        .access = static_cast<u8>(access),
        .limit_high = static_cast<u8>((limit >> 16) & 0x0f),
        .flags = static_cast<u8>(flags),
        .base_high = static_cast<u8>((base >> 24) & 0xff),
    };
}

void initialize()
{
    s_entries[0] = create_entry(0x00000000, 0x00000000, ACCESS_ATTRIBUTE_NONE, FLAG_ATTRIBUTE_NONE);

    s_entries[1] = create_entry(
        0x00000000,
        0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE
            | ACCESS_ATTRIBUTE_READ_WRITE,
        FLAG_ATTRIBUTE_NONE);
    s_entries[2] = create_entry(
        0x00000000,
        0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_NONE);

    s_entries[3] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE
            | ACCESS_ATTRIBUTE_READ_WRITE,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_32);
    s_entries[4] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_32);

    s_entries[5] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE
            | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_64);
    s_entries[6] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_64);

    s_descriptor.size = sizeof(s_entries) - 1;
    s_descriptor.address = reinterpret_cast<u64>(s_entries);

    load_gdt(&s_descriptor);
    reload_segments();

    logger::ok("GDT: Initialized\n");
}

} // namespace kernel::gdt
