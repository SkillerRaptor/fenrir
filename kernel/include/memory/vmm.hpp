/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/bitflags.hpp"
#include "lib/types.hpp"

namespace vmm {

enum class Attribute : u16 {
    None = 0 << 0,
    Present = 1 << 0,
    Write = 1 << 1,
    User = 1 << 2,
};

DECLARE_BITFLAG(Attribute);

struct PageMap {
    u64 top_level = 0;
};

void initialize();

PageMap *create_page_map();
void destroy_page_map(const PageMap *);
void switch_to_page_map(const PageMap *);

void map(const PageMap *, u64 paddr, u64 vaddr, Attribute attributes);
void unmap(const PageMap *, u64 vaddr);

u64 virtual_to_physical(const PageMap *, u64 vaddr);

PageMap *get_kernel_page_map();

} // namespace vmm
