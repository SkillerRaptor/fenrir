/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/gdt.hpp"

#include "kernel/core/bitflags.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/types.hpp"

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

static Entry s_entries[9] = { };
static Descriptor s_descriptor = { };

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

void initialize()
{
    s_entries[0] = create_entry(0x00000000, 0x00000000, AccessAttribute::None, FlagAttribute::None);

    s_entries[1] = create_entry(
        0x00000000,
        0x0000ffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite,
        FlagAttribute::None);
    s_entries[2] = create_entry(
        0x00000000,
        0x0000ffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::None);

    s_entries[3] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite,
        FlagAttribute::PageGranularity | FlagAttribute::Size32);
    s_entries[4] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::Size32);

    s_entries[5] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);
    s_entries[6] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::KernelPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);

    s_entries[7] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::UserPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::Executable | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);
    s_entries[8] = create_entry(
        0x00000000,
        0xffffffff,
        AccessAttribute::Present | AccessAttribute::UserPrivilege | AccessAttribute::CodeDate
            | AccessAttribute::ReadWrite | AccessAttribute::Access,
        FlagAttribute::PageGranularity | FlagAttribute::LongMode);

    s_descriptor.size = sizeof(s_entries) - 1;
    s_descriptor.address = reinterpret_cast<u64>(s_entries);

    load();

    logger::info("GDT: Initialized\n");
}

void load()
{
    load_gdt(&s_descriptor);
    reload_segments();
}

} // namespace kernel::gdt
