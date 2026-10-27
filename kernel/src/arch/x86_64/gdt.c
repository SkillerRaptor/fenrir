/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/gdt.h"

#include <stdint.h>

#include "lib/log.h"

enum AccessAttribute : uint8_t {
    ACCESS_ATTRIBUTE_NONE = 0,
    ACCESS_ATTRIBUTE_ACCESSED = 1 << 0,
    ACCESS_ATTRIBUTE_READABLE_WRITABLE = 1 << 1,
    ACCESS_ATTRIBUTE_DIRECTION = 1 << 2,
    ACCESS_ATTRIBUTE_EXECUTABLE = 1 << 3,
    ACCESS_ATTRIBUTE_CODE_OR_DATA = 1 << 4,
    ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE = 0 << 6 | 0 << 5,
    ACCESS_ATTRIBUTE_USER_PRIVILEGE = 1 << 6 | 1 << 5,
    ACCESS_ATTRIBUTE_PRESENT = 1 << 7,
};

enum FlagAttribute : uint8_t {
    FLAG_ATTRIBUTE_NONE = 0,
    FLAG_ATTRIBUTE_LONG_MODE = 1 << 1,
    FLAG_ATTRIBUTE_SIZE_32 = 1 << 2,
    FLAG_ATTRIBUTE_PAGE_GRANULARITY = 1 << 3,
};

struct [[gnu::packed]] Entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
};

static_assert(sizeof(struct Entry) == 8);

struct [[gnu::packed]] Descriptor {
    uint16_t size;
    uint64_t address;
};

static_assert(sizeof(struct Descriptor) == 10);

struct Table {
    struct Entry entries[7];
};

static struct Table s_table = { };

extern void load_gdt(struct Descriptor *descriptor);
extern void reload_segments();

static struct Entry create_entry(uint32_t base, uint32_t limit, enum AccessAttribute access, enum FlagAttribute flags)
{
    return (struct Entry) {
        .limit_low = (uint16_t) limit,
        .base_low = (uint16_t) base,
        .base_middle = (uint8_t) (base >> 16),
        .access = access,
        .limit_high_flags = (uint8_t) (((flags << 4) & 0xf0) | ((limit >> 16) & 0x0f)),
        .base_high = (uint8_t) (base >> 24),
    };
}

void gdt_init()
{
    s_table.entries[0] = create_entry(0x00000000, 0x00000000, ACCESS_ATTRIBUTE_NONE, FLAG_ATTRIBUTE_NONE);

    s_table.entries[1] = create_entry(
        0x00000000,
        0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_NONE);
    s_table.entries[2] = create_entry(
        0x00000000,
        0x0000ffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_NONE);

    s_table.entries[3] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_PAGE_GRANULARITY | FLAG_ATTRIBUTE_SIZE_32);
    s_table.entries[4] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_PAGE_GRANULARITY | FLAG_ATTRIBUTE_SIZE_32);

    s_table.entries[5] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_EXECUTABLE | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_PAGE_GRANULARITY | FLAG_ATTRIBUTE_LONG_MODE);
    s_table.entries[6] = create_entry(
        0x00000000,
        0xffffffff,
        ACCESS_ATTRIBUTE_PRESENT | ACCESS_ATTRIBUTE_KERNEL_PRIVILEGE | ACCESS_ATTRIBUTE_CODE_OR_DATA
            | ACCESS_ATTRIBUTE_READABLE_WRITABLE | ACCESS_ATTRIBUTE_ACCESSED,
        FLAG_ATTRIBUTE_PAGE_GRANULARITY | FLAG_ATTRIBUTE_LONG_MODE);

    struct Descriptor descriptor = {
        .size = sizeof(s_table) - 1,
        .address = (uintptr_t) &s_table,
    };

    load_gdt(&descriptor);
    reload_segments();

    log_info("GDT: Initialized\n");
}
