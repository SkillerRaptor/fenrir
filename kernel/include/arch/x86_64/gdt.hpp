/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace kernel::gdt {

struct Tss {
    u32 reserved_0;
    u64 rsp_0;
    u64 rsp_1;
    u64 rsp_2;
    u64 reserved_1;
    u64 ist_1;
    u64 ist_2;
    u64 ist_3;
    u64 ist_4;
    u64 ist_5;
    u64 ist_6;
    u64 ist_7;
    u64 reserved_2;
    u16 reserved_3;
    u16 io_offset;
} __attribute__((__packed__));

struct Entry {
    u16 limit_low { 0 };
    u16 base_low { 0 };
    u8 base_middle { 0 };
    u8 access { 0 };
    u8 limit_high : 4 { 0 };
    u8 flags : 4 { 0 };
    u8 base_high { 0 };
} __attribute__((packed));

struct TssEntry {
    u16 limit_low { 0 };
    u16 base_low { 0 };
    u8 base_middle_1 { 0 };
    u8 access { 0 };
    u8 limit_high : 4 { 0 };
    u8 flags : 4 { 0 };
    u8 base_middle_2 { 0 };
    u32 base_high { 0 };
    u32 reserved { 0 };
} __attribute__((packed));

struct Table {
    Entry entries[9] {};
    TssEntry tss_entry {};
} __attribute__((packed));

struct Descriptor {
    u16 size { 0 };
    u64 address { 0 };
} __attribute__((packed));

struct CpuGdt {
    Table table {};
    Descriptor descriptor {};
} __attribute__((packed));

Table create_table();
void load(CpuGdt &, const Tss &);

} // namespace kernel::gdt
