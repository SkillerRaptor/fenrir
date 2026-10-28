/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "mem/pmm.h"

#include <stddef.h>
#include <stdint.h>

#include "boot/boot.h"
#include "lib/assertion.h"
#include "lib/log.h"
#include "lib/math.h"
#include "lib/string.h"
#include "limine.h"
#include "mem/bitmap.h"
#include "mem/page.h"

static uint64_t s_highest_address = 0;
static size_t s_total_pages = 0;
static struct Bitmap s_bitmap = { };

void pmm_init()
{
    const struct limine_memmap_response *memory_map_response = boot_get_memory_map_response();

    LOG_DEBUG("PMM: Scanning %lu memory map entries\n", memory_map_response->entry_count);

    for (size_t i = 0; i < memory_map_response->entry_count; ++i) {
        const struct limine_memmap_entry *entry = memory_map_response->entries[i];

        const char *type = "<unknown>";
        switch (entry->type) {
        case LIMINE_MEMMAP_USABLE:
            type = "Usable";
            break;
        case LIMINE_MEMMAP_RESERVED:
            type = "Reserved";
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            type = "ACPI Reclaimable";
            break;
        case LIMINE_MEMMAP_ACPI_NVS:
            type = "ACPI NVS";
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            type = "Bad Memory";
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            type = "Bootloader Reclaimable";
            break;
        case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
            type = "Executable and Modules";
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            type = "Framebuffer";
            break;
        case LIMINE_MEMMAP_RESERVED_MAPPED:
            type = "Reserved Mapped";
            break;
        }

        LOG_DEBUG("PMM:   [%#018lx-%#018lx] %s\n", entry->base, entry->base + entry->length, type);

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        const uint64_t end_of_page = entry->base + entry->length;
        if (end_of_page > s_highest_address) {
            s_highest_address = end_of_page;
        }
    }

    VERIFY(s_highest_address != 0, "Failed to find usable region in memory map");

    s_total_pages = math_div_round_up(s_highest_address, PAGE_SIZE);

    const size_t bitmap_bytes = math_div_round_up(s_total_pages, 8);
    const size_t bitmap_pages = math_div_round_up(bitmap_bytes, PAGE_SIZE);

    uint64_t bitmap_base = 0;
    bool bitmap_found = false;
    for (size_t i = 0; i < memory_map_response->entry_count; ++i) {
        const struct limine_memmap_entry *entry = memory_map_response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE || entry->length < bitmap_pages * 8) {
            continue;
        }

        bitmap_base = entry->base;
        bitmap_found = true;
        break;
    }

    VERIFY(bitmap_found, "Failed to find usable region to hold the bitmap");

    s_bitmap.data = (uint8_t *) (bitmap_base + boot_get_hhdm_offset());
    s_bitmap.size = bitmap_bytes;

    LOG_DEBUG(
        "PMM: Placed bitmap at %#018lx with a size of %lu (%lu pages)\n",
        (uintptr_t) s_bitmap.data,
        s_bitmap.size,
        bitmap_pages);

    bitmap_fill(&s_bitmap, 0xff);

    size_t free_pages = 0;
    for (size_t i = 0; i < memory_map_response->entry_count; ++i) {
        const struct limine_memmap_entry *entry = memory_map_response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        const size_t first_page = entry->base / PAGE_SIZE;
        const size_t page_count = entry->length / PAGE_SIZE;

        for (size_t page = first_page; page < first_page + page_count; ++page) {
            bitmap_set(&s_bitmap, page, false);
        }

        free_pages += page_count;
    }

    const size_t bitmap_first_page = bitmap_base / PAGE_SIZE;
    for (size_t page = bitmap_first_page; page < bitmap_first_page + bitmap_pages; ++page) {
        bitmap_set(&s_bitmap, page, true);
    }

    free_pages -= bitmap_pages;

    LOG_DEBUG(
        "PMM: Detected %lu free pages (%lu MiB) out of %lu\n",
        free_pages,
        (free_pages * PAGE_SIZE) / (1024 * 1024),
        s_total_pages);

    LOG_INFO("PMM: Initialized\n");
}

void *pmm_alloc(size_t page_count)
{
    if (page_count == 0) {
        return nullptr;
    }

    size_t current_pages = 0;
    for (size_t i = 0; i < s_total_pages; ++i) {
        if (bitmap_is_set(&s_bitmap, i)) {
            current_pages = 0;
            continue;
        }

        ++current_pages;

        if (current_pages != page_count) {
            continue;
        }

        const size_t start_page = i + 1 - page_count;
        for (size_t page = start_page; page < start_page + page_count; ++page) {
            bitmap_set(&s_bitmap, page, true);
        }

        return (void *) ((uint64_t) start_page * PAGE_SIZE);
    }

    return nullptr;
}

void *pmm_calloc(size_t page_count)
{
    if (page_count == 0) {
        return nullptr;
    }

    void *ptr = pmm_alloc(page_count);
    if (ptr == nullptr) {
        return nullptr;
    }

    void *virtual_ptr = (void *) ((uint64_t) ptr + boot_get_hhdm_offset());
    memset(virtual_ptr, 0x00, page_count * PAGE_SIZE);

    return ptr;
}

void pmm_free(void *ptr, size_t page_count)
{
    if (ptr == nullptr || page_count == 0) {
        return;
    }

    const size_t first_page = (uint64_t) ptr / PAGE_SIZE;
    for (size_t i = 0; i < page_count; ++i) {
        bitmap_set(&s_bitmap, first_page + i, false);
    }
}
