/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <ygg/types.hpp>

namespace memory {

void *kmalloc(usize size);
void kfree(void *ptr);

} // namespace memory
