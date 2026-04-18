/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "memory/pmm.hpp"

#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "lib/assert.hpp"
#include "lib/bitmap.hpp"
#include "lib/math.hpp"
#include "lib/string.hpp"

namespace pmm {

static usize s_highest_page = 0;
static Bitmap s_bitmap { };

void initialize()
{
    const Span<limine_memmap_entry *> memory_map = boot::get_memory_map();

    logger::debug("PMM: Scanning %zu memory map entries\n", memory_map.size());

    for (limine_memmap_entry *entry : memory_map) {
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

        logger::debug("PMM:   [0x%016llx - 0x%016llx] - %s\n", entry->base, entry->base + entry->length, type);

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

    logger::debug("PMM: Found highest page address at 0x%llx\n", s_highest_page);

    s_bitmap.set_size(math::div_round_up(s_highest_page, memory::s_page_size));

    for (const limine_memmap_entry *entry : memory_map) {
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length >= s_bitmap.size()) {
            s_bitmap.set_data(reinterpret_cast<u8 *>(entry->base + boot::get_hhdm_offset()));
            memset(s_bitmap.data(), 0xff, s_bitmap.size() / 8);
            break;
        }
    }

    logger::debug(
        "PMM: Placed bitmap at 0x%p with a size of %zu bytes (%zu KiB)\n",
        s_bitmap.data(),
        s_bitmap.size() / 8,
        s_bitmap.size() / 8 / 1024);

    usize free_pages = 0;

    for (const limine_memmap_entry *entry : memory_map) {
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (reinterpret_cast<u8 *>(entry->base + boot::get_hhdm_offset()) == s_bitmap.data()) {
            const usize remaining_bytes = entry->length - s_bitmap.size();
            for (usize j { 0 }; j < remaining_bytes; j += memory::s_page_size) {
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
            s_bitmap.set(page_index, false); // Here
        }

        free_pages += entry->length / memory::s_page_size;
    }

    logger::debug(
        "PMM: Detected %zu free pages (%zu KiB, %zu MiB)\n",
        free_pages,
        (free_pages * memory::s_page_size) / 1024,
        (free_pages * memory::s_page_size) / 1024 / 1024);

    logger::info("PMM: Initialized\n");
}

void *allocate(const usize pages, const bool clear)
{
    assert(pages > 0);

    usize current_pages = 0;
    for (usize i = 0; i < s_highest_page / memory::s_page_size; ++i) {
        if (s_bitmap.get(i)) {
            current_pages = 0;
            continue;
        }

        if (++current_pages != pages) {
            continue;
        }

        const usize index = i + 1;
        const usize page = index - pages;
        for (usize j = page; j < index; ++j) {
            s_bitmap.set(j, true);
        }

        void *ptr = reinterpret_cast<void *>(page * memory::s_page_size);

        if (clear) {
            u64 *address = reinterpret_cast<u64 *>(reinterpret_cast<u64>(ptr) + boot::get_hhdm_offset());

            for (usize current_page = 0; current_page < pages * (memory::s_page_size / sizeof(u64)); ++current_page) {
                address[current_page] = 0;
            }
        }

        return ptr;
    }

    logger::err("PMM: Out of memory - failed to allocate %zu pages\n", pages);

    return nullptr;
}

void free(void *ptr, const usize pages)
{
    const usize address = reinterpret_cast<usize>(ptr);
    const usize page = address / memory::s_page_size;

    for (usize i = 0; i < pages; ++i) {
        s_bitmap.set(page + i, false);
    }
}

} // namespace pmm
