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
    logger::info("PMM: Initializing...\n");

    const usize memory_map_entry_count = boot::get_memory_map_entry_count();

    logger::info("PMM: Scanning %zu memory map entries\n", memory_map_entry_count);

    for (usize i = 0; i < memory_map_entry_count; ++i) {
        limine_memmap_entry *entry = boot::get_memory_map_entry(i);

        const char *type = [&entry]() {
            switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                return "Usable";
            case LIMINE_MEMMAP_RESERVED:
                return "Reserved";
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                return "ACPI Reclaimable";
            case LIMINE_MEMMAP_ACPI_NVS:
                return "ACPI NVS";
            case LIMINE_MEMMAP_BAD_MEMORY:
                return "Bad Memory";
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                return "Bootloader Reclaimable";
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                return "Executable and Modules";
            case LIMINE_MEMMAP_FRAMEBUFFER:
                return "Framebuffer";
            case LIMINE_MEMMAP_RESERVED_MAPPED:
                return "Reserved Mapped";
            default:
                return "<unknown>";
            }
        }();

        logger::info("PMM:   %02u: [%016llx - %016llx] - %s\n", i, entry->base, entry->base + entry->length, type);

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

    logger::info("PMM: Found highest page address at 0x%llx\n", s_highest_page);

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

    logger::info(
        "PMM: Placed bitmap at 0x%p with a size of %zu bytes (%zu KiB)\n",
        s_bitmap.data(),
        s_bitmap.size(),
        s_bitmap.size() / 1024);

    usize free_pages = 0;
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

            free_pages += remaining_bytes / memory::s_page_size;

            continue;
        }

        for (usize j = 0; j < entry->length; j += memory::s_page_size) {
            const usize address = entry->base + j;
            const usize page_index = address / memory::s_page_size;
            s_bitmap.set(page_index, false);
        }

        free_pages += entry->length / memory::s_page_size;
    }
    logger::info(
        "PMM: Detected %zu free pages (%zu KiB, %zu MiB)\n",
        free_pages,
        (free_pages * memory::s_page_size) / 1024,
        (free_pages * memory::s_page_size) / 1024 / 1024);

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

    if (!ptr) {
        logger::err("PMM: Out of memory - failed to allocate %zu pages\n", pages);
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
