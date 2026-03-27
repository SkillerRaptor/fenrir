/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

namespace kernel::memory {

extern "C" void *memcpy(void *dst, const void *src, usize count);
extern "C" void *memset(void *dst, int c, usize count);

} // namespace kernel::memory
