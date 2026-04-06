/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/gdt.hpp"

#include "lib/bitflags.hpp"
#include "lib/types.hpp"

namespace kernel::gdt {

enum class AccessAttribute : u8 {
    None = 0,
    Access = 1 << 0,
    ReadWrite = 1 << 1,
    Direction = 1 << 2,
    Executable = 1 << 3,
    CodeDate = 1 << 4,
    KernelPrivilege = 0 << 6 | 0 << 5,
    UserPrivilege = 1 << 6 | 1 << 5,
    Present = 1 << 7,
};

DECLARE_BITFLAG(AccessAttribute);

enum class FlagAttribute : u8 {
    None = 0,
    LongMode = 1 << 1,
    Size32 = 1 << 2,
    PageGranularity = 1 << 3,
};

DECLARE_BITFLAG(FlagAttribute);

extern "C" void load_gdt(const Descriptor *descriptor);
extern "C" void load_tss();
extern "C" void reload_segments();

static Entry create_entry(const u32 base, const u32 limit, const AccessAttribute access, const FlagAttribute flags)
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

static TssEntry create_tss_entry(const u64 address)
{
    constexpr u32 limit = sizeof(Tss);

    return {
        .limit_low = static_cast<u16>(limit & 0xffff),
        .base_low = static_cast<u16>(address & 0xffff),
        .base_middle_1 = static_cast<u8>((address >> 16) & 0xff),
        .access = static_cast<u8>(
            AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::Executable
            | AccessAttribute::Access),
        .limit_high = static_cast<u8>((limit >> 16) & 0x0f),
        .flags = static_cast<u8>(FlagAttribute::PageGranularity) | 0b1,
        .base_middle_2 = static_cast<u8>((address >> 24) & 0xff),
        .base_high = static_cast<u32>((address >> 32) & 0xffffffff),
        .reserved = static_cast<u32>(0),
    };
}

Table create_table()
{
    Table table {};

    table.entries[0] = create_entry(0x00000000, 0x00000000, AccessAttribute::None, FlagAttribute::None);

    table.entries[1] = create_entry(
        0x00000000,
        0x0000ffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite,
        FlagAttribute::None);
    table.entries[2] = create_entry(
        0x00000000,
        0x0000ffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::None);

    table.entries[3] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite,
        FlagAttribute::PageGranularity | FlagAttribute::Size32);
    table.entries[4] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::Size32);

    table.entries[5] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);
    table.entries[6] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);

    table.entries[8] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::UserPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);
    table.entries[7] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::UserPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);

    return table;
}

void load(CpuGdt &cpu_gdt, const Tss &tss)
{
    cpu_gdt.table.tss_entry = create_tss_entry(reinterpret_cast<u64>(&tss));

    cpu_gdt.descriptor = {
        .size = sizeof(cpu_gdt.table) - 1,
        .address = reinterpret_cast<u64>(&cpu_gdt.table),
    };

    load_gdt(&cpu_gdt.descriptor);
    reload_segments();

    load_tss();
}

} // namespace kernel::gdt
