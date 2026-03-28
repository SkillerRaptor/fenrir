/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "kernel/core/types.hpp"

namespace kernel::vmm {

#define ATTRIBUTE_PRESENT (1 << 0)
#define ATTRIBUTE_WRITE (1 << 1)
#define ATTRIBUTE_USER (1 << 2)

struct PageMap {
    u64 top_level { 0 };
};

void initialize();

PageMap *create_page_map();
void switch_to_page_map(const PageMap *);

void map(const PageMap *, u64 paddr, u64 vaddr, u16 flags);
void unmap(const PageMap *, u64 vaddr);

PageMap *get_kernel_page_map();

} // namespace kernel::vmm
