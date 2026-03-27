/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/gdt.hpp"

#include <stdint.h>

namespace gdt {

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
    uint16_t limit_low { 0 };
    uint16_t base_low { 0 };
    uint8_t base_middle { 0 };
    uint8_t access { 0 };
    uint8_t limit_high : 4 { 0 };
    uint8_t flags : 4 { 0 };
    uint8_t base_high { 0 };
} __attribute__((packed));

struct Descriptor {
    uint16_t size { 0 };
    uint64_t address { 0 };
} __attribute__((packed));

extern "C" void load_gdt(const Descriptor *descriptor);
extern "C" void reload_segments();

static Entry s_entries[7] { };
static Descriptor s_descriptor { };

static Entry create_entry(const uint32_t base, const uint32_t limit, const uint8_t access, const uint8_t flags)
{
    return {
        .limit_low = static_cast<uint16_t>(limit & 0xffff),
        .base_low = static_cast<uint16_t>(base & 0xffff),
        .base_middle = static_cast<uint8_t>((base >> 16) & 0xff),
        .access = static_cast<uint8_t>(access),
        .limit_high = static_cast<uint8_t>((limit >> 16) & 0x0f),
        .flags = static_cast<uint8_t>(flags),
        .base_high = static_cast<uint8_t>((base >> 24) & 0xff),
    };
}

void initialize()
{
    s_entries[0] = create_entry(0x00000000, 0x00000000, ACCESS_ATTRIBUTE_NONE, FLAG_ATTRIBUTE_NONE);

    s_entries[1] = create_entry(
        0x00000000, 0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READ_WRITE,
        FLAG_ATTRIBUTE_NONE);
    s_entries[2] = create_entry(
        0x00000000, 0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_NONE);

    s_entries[3] = create_entry(
        0x00000000, 0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READ_WRITE,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_32);
    s_entries[4] = create_entry(
        0x00000000, 0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_32);

    s_entries[5] = create_entry(
        0x00000000, 0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READ_WRITE
            | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_64);
    s_entries[6] = create_entry(
        0x00000000, 0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_CODE_DATA | ACCESS_ATTRIBUTE_READ_WRITE | ACCESS_ATTRIBUTE_ACCESS,
        FLAG_ATTRIBUTE_4K | FLAG_ATTRIBUTE_64);

    s_descriptor.size = sizeof(s_entries) - 1;
    s_descriptor.address = reinterpret_cast<uint64_t>(&s_entries);

    load_gdt(&s_descriptor);
    reload_segments();
}

} // namespace gdt
