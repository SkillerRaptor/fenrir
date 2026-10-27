/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "mem/kmalloc.h"

#include <stdint.h>

#include "boot/boot.h"
#include "lib/math.h"
#include "lib/string.h"
#include "mem/page.h"
#include "mem/pmm.h"

struct AllocationHeader {
    size_t page_count;
    size_t size;
};

void *kmalloc(size_t size)
{
    if (size == 0) {
        return nullptr;
    }

    const size_t page_count = math_div_round_up(size, PAGE_SIZE);

    void *physical_ptr = pmm_alloc(page_count + 1);
    if (!physical_ptr) {
        return nullptr;
    }

    uint8_t *ptr = (uint8_t *) ((uintptr_t) physical_ptr + boot_get_hhdm_offset());

    struct AllocationHeader *header = (struct AllocationHeader *) ptr;
    header->page_count = page_count;
    header->size = size;

    return ptr + PAGE_SIZE;
}

void *kcalloc(size_t count, size_t size)
{
    if (count == 0 || size == 0) {
        return nullptr;
    }

    const size_t total_size = count * size;

    void *ptr = kmalloc(total_size);
    if (!ptr) {
        return nullptr;
    }

    memset(ptr, 0x00, total_size);

    return ptr;
}

void kfree(void *ptr)
{
    if (!ptr) {
        return;
    }

    uint8_t *header_address = (uint8_t *) ptr - PAGE_SIZE;
    const struct AllocationHeader *header = (const struct AllocationHeader *) header_address;

    void *physical_ptr = (void *) ((uintptr_t) header_address - boot_get_hhdm_offset());
    pmm_free(physical_ptr, header->page_count + 1);
}
