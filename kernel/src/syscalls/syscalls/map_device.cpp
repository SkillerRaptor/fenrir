/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "core/memory.hpp"
#include "lib/math.hpp"
#include "memory/vmm.hpp"
#include "scheduler/process.hpp"
#include "syscall/syscalls.hpp"

namespace syscalls {

u64 sys$map_device(const Span<const u64> arguments)
{
    const cpu::Core &core = cpu::current();

    limine_framebuffer *framebuffer = boot::get_framebuffers()[0];

    const u64 physical_address = reinterpret_cast<u64>(framebuffer->address) - boot::get_hhdm_offset();
    const u64 byte_size = framebuffer->pitch * framebuffer->height;

    const u64 aligned_physical_address = math::align_down(physical_address, memory::s_page_size);
    const u64 physical_offset = physical_address - aligned_physical_address;

    const u64 total_bytes = byte_size + physical_offset;
    const u64 total_pages = math::div_round_up(total_bytes, memory::s_page_size);

    for (u64 i = 0; i < total_pages; ++i) {
        vmm::map(
            core.current_thread->process->page_map,
            aligned_physical_address + i * memory::s_page_size,
            0x1000000 + i * memory::s_page_size,
            vmm::Attribute::Write | vmm::Attribute::User);
    }

    u64 *ptr = reinterpret_cast<u64 *>(arguments[1]);
    *ptr = 0x1000000;

    return 0;
}

} // namespace syscalls
