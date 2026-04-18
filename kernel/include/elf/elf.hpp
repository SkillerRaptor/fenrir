/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/bitflags.hpp"
#include "lib/types.hpp"
#include "lib/vector.hpp"

namespace elf {

enum class Format : u8 {
    _32 = 1,
    _64 = 2,
};

enum class Endianness : u8 {
    Little = 1,
    Big = 2,
};

struct Header {
    enum class Type : u16 {
        None = 0x0,
        Relocatable = 0x1,
        Executable = 0x2,
        Dynamic = 0x3,
        Core = 0x4,
    };

    u8 magic[4] { };
    Format format = Format::_64;
    Endianness endianness = Endianness::Little;
    u8 version = 0;
    u8 os_abi = 0;
    u8 abi_version = 0;
    u8 padding[7] { };
    Type type = Type::None;
    u16 machine = 0;
    u32 elf_version = 0;
    u64 entry = 0;
    u64 program_header_offset = 0;
    u64 section_header_offset = 0;
    u32 flags = 0;
    u16 header_size = 0;
    u16 program_header_size = 0;
    u16 program_header_entry_count = 0;
    u16 section_header_size = 0;
    u16 section_header_entry_count = 0;
    u16 section_name_index = 0;
} __attribute__((packed));

struct ProgramHeader {
    enum class Type : u32 {
        Null = 0x00000000,
        Load = 0x00000001,
        Dynamic = 0x00000002,
        Interpreter = 0x00000003,
        Note = 0x00000004,
        Reserved = 0x00000005,
        ProgramHeader = 0x00000006,
        ThreadLocalStorage = 0x00000007,
    };

    enum class Flags : u32 {
        None = 0,
        Executable = 1 << 0,
        Writeable = 1 << 1,
        Readable = 1 << 2,
    };

    Type type = Type::Null;
    Flags flags = Flags::None;
    u64 offset = 0;
    u64 virtual_address = 0;
    u64 physical_address = 0;
    u64 file_size = 0;
    u64 memory_size = 0;
    u64 align = 0;
} __attribute__((packed));

DECLARE_BITFLAG(ProgramHeader::Flags);

struct SectionHeader {
    enum class Type : u32 {
        Null = 0x0,
        ProgramData = 0x1,
        SymbolTable = 0x2,
        StringTable = 0x3,
        RelocationAddends = 0x4,
        Hash = 0x5,
        Dynamic = 0x6,
        Note = 0x7,
        NoData = 0x8,
        Relocation = 0x9,
        Reserved = 0xa,
        DynamicSymbol = 0xb,
    };

    enum class Flags : u64 {
        None = 0,
        Write = 1 << 0,
        Allocation = 1 << 1,
        Executable = 1 << 2,
        Merge = 1 << 3,
        Strings = 1 << 4,
        InfoLink = 1 << 5,
        LinkOrder = 1 << 6,
        OsNonConforming = 1 << 7,
        Group = 1 << 8,
        ThreadLocalStorage = 1 << 9,
    };

    u32 name = 0;
    Type type = Type::Null;
    Flags flags = Flags::None;
    u64 virtual_address = 0;
    u64 offset = 0;
    u64 size = 0;
    u32 link = 0;
    u32 info = 0;
    u64 address_alignment = 0;
    u64 entry_size = 0;
} __attribute__((packed));

DECLARE_BITFLAG(SectionHeader::Flags);

class Elf {
private:
    static constexpr u8 s_magic[4] = { 0x7f, 'E', 'L', 'F' };

public:
    // TODO: Use a factory function and return optional
    explicit Elf(const u8 *data);

    const Header &header() const { return m_header; }
    const Vector<ProgramHeader> &program_headers() const { return m_program_headers; }
    const Vector<SectionHeader> &section_headers() const { return m_section_headers; }

private:
    // TODO: Don't save header
    Header m_header { };
    Vector<ProgramHeader> m_program_headers { };
    Vector<SectionHeader> m_section_headers { };
};

} // namespace elf
