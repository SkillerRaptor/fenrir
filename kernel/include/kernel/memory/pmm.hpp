/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lib/types.hpp>

namespace kernel::pmm {

void initialize();

void *allocate(usize pages, bool clear);
void free(void *ptr, usize pages);

} // namespace kernel::pmm
