/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

enum PageAttribute : uint16_t {
    PAGE_ATTRIBUTE_NONE = 0,
    PAGE_ATTRIBUTE_PRESENT = 1 << 0,
    PAGE_ATTRIBUTE_WRITE = 1 << 1,
    PAGE_ATTRIBUTE_USER = 1 << 2,
};

struct PageMap {
    uint64_t *top_level;
};

void vmm_init();

struct PageMap *vmm_create_page_map();
void vmm_switch_page_map(struct PageMap *page_map);

struct PageMap *vmm_get_kernel_page_map();

void vmm_map_page(struct PageMap *page_map, uint64_t phys_addr, uint64_t virt_addr, enum PageAttribute attributes);
void vmm_unmap_page(struct PageMap *page_map, uint64_t virt_addr);
