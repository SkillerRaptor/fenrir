/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/memory/pmm.hpp"

#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/memory.hpp"
#include "kernel/memory/bitmap.hpp"

namespace kernel::pmm {

static usize s_highest_page { 0 };
static usize s_last_used_index { 0 };
static Bitmap s_bitmap { nullptr, 0 };

void initialize()
{
    const usize memory_map_entry_count = boot::get_memory_map_entry_count();

    for (usize i = 0; i < memory_map_entry_count; ++i) {
        limine_memmap_entry *entry = boot::get_memory_map_entry(i);

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        // NOTE: Remove first page from memory map
        if (entry->base == 0) {
            entry->base += memory::s_page_size;
            entry->length -= memory::s_page_size;
        }

        const usize end_of_page = entry->base + entry->length;
        if (end_of_page > s_highest_page) {
            s_highest_page = end_of_page;
        }
    }

    s_bitmap.set_size(memory::div_round_up(s_highest_page, memory::s_page_size) / 8);

    for (usize i = 0; i < memory_map_entry_count; ++i) {
        const limine_memmap_entry *entry = boot::get_memory_map_entry(i);

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= s_bitmap.size()) {
            s_bitmap.set_data(reinterpret_cast<u8 *>(entry->base + boot::get_hhdm_offset()));
            memory::memset(s_bitmap.data(), 0xff, s_bitmap.size());
            break;
        }
    }

    for (usize i = 0; i < memory_map_entry_count; ++i) {
        const limine_memmap_entry *entry = boot::get_memory_map_entry(i);

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (reinterpret_cast<u8 *>(entry->base + boot::get_hhdm_offset()) == s_bitmap.data()) {
            const usize remaining_bytes = entry->length - s_bitmap.size();
            for (usize j = 0; j < remaining_bytes; j += memory::s_page_size) {
                const usize address = (entry->base + s_bitmap.size()) + j;
                const usize page_index = address / memory::s_page_size;
                s_bitmap.set(page_index, false);
            }

            continue;
        }

        for (usize j = 0; j < entry->length; j += memory::s_page_size) {
            const usize address = entry->base + j;
            const usize page_index = address / memory::s_page_size;
            s_bitmap.set(page_index, false);
        }
    }

    logger::ok("PMM: Initialized\n");
}

static void *internal_allocate(const usize pages, const usize limit)
{
    if (pages == 0) {
        return nullptr;
    }

    usize current_pages = 0;
    while (s_last_used_index < limit) {
        if (s_bitmap.get(s_last_used_index++)) {
            current_pages = 0;
            continue;
        }

        if (++current_pages != pages) {
            continue;
        }

        const usize page = s_last_used_index - pages;
        for (usize i = page; i < s_last_used_index; ++i) {
            s_bitmap.set(i, true);
        }

        return reinterpret_cast<void *>(page * memory::s_page_size);
    }

    return nullptr;
}

void *allocate(const usize pages)
{
    const usize limit = s_last_used_index;
    void *ptr = internal_allocate(pages, s_highest_page / memory::s_page_size);
    if (!ptr) {
        s_last_used_index = 0;
        ptr = internal_allocate(pages, limit);
    }

    return ptr;
}

void free(void *ptr, const usize pages)
{
    const usize address = reinterpret_cast<usize>(ptr);
    const usize page = address / memory::s_page_size;

    for (usize i = 0; i < pages; ++i) {
        s_bitmap.set(page + i, false);
    }
}

} // namespace kernel::pmm
