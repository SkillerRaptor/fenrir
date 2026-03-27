/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "kernel/core/types.hpp"

namespace kernel::pmm {

void initialize();

void *allocate(usize pages);
void free(void *ptr, usize pages);

} // namespace kernel::pmm
