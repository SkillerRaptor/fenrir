/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/memory/kmalloc.hpp"

#include "kernel/core/boot.hpp"
#include "kernel/core/memory.hpp"
#include "kernel/memory/pmm.hpp"

namespace kernel::memory {

struct AllocationHeader {
    usize page_count { 0 };
    usize size { 0 };
};

void *kmalloc(const usize size)
{
    const usize page_count = div_round_up(size, s_page_size);

    // NOTE: Allocate one extra page for the header
    u8 *ptr = static_cast<u8 *>(pmm::allocate(page_count + 1));
    if (!ptr) {
        return nullptr;
    }

    ptr += boot::get_hhdm_offset();

    AllocationHeader *header = reinterpret_cast<struct AllocationHeader *>(ptr);
    ptr += s_page_size;

    header->page_count = page_count;
    header->size = size;

    return ptr;
}

void kfree(const void *ptr)
{
    if (!ptr) {
        return;
    }

    const u8 *header_address = static_cast<const u8 *>(ptr) - s_page_size;
    const AllocationHeader *header = reinterpret_cast<const AllocationHeader *>(header_address);

    const usize page_count = header->page_count + 1;
    pmm::free(reinterpret_cast<void *>(reinterpret_cast<u64>(header_address) - boot::get_hhdm_offset()), page_count);
}

} // namespace kernel::memory
