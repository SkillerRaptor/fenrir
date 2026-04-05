/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lib/types.hpp>

namespace kernel::memory {

void *kmalloc(usize size);
void kfree(void *ptr);

} // namespace kernel::memory
