/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "elf/elf.hpp"

#include <ygg/assert.hpp>
#include <ygg/string.hpp>

namespace elf {

Elf::Elf(const u8 *data)
{
    ASSERT(data);

    memcpy(&m_header, data, sizeof(Header));

    ASSERT(memcmp(m_header.magic, s_magic, 4) == 0);
    ASSERT(m_header.format == Format::_64);
    ASSERT(m_header.machine == 0x3e); // NOTE: This is x86_64

    for (u16 i { 0 }; i < m_header.program_header_entry_count; ++i) {
        const ProgramHeader *program_header = reinterpret_cast<const ProgramHeader *>(
            data + m_header.program_header_offset + i * m_header.program_header_size);
        m_program_headers.push_back(*program_header);
    }

    for (u16 i { 0 }; i < m_header.section_header_entry_count; ++i) {
        const SectionHeader *section_header = reinterpret_cast<const SectionHeader *>(
            data + m_header.section_header_offset + i * m_header.section_header_size);
        m_section_headers.push_back(*section_header);
    }
}

} // namespace elf
