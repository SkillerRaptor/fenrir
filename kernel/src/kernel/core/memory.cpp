/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/core/memory.hpp"

namespace kernel::memory {

void *memcpy(void *dst, const void *src, const usize count)
{
    u8 *dst_ptr = static_cast<u8 *>(dst);
    const u8 *src_ptr = static_cast<const u8 *>(src);

    for (usize i = 0; i < count; i++) {
        dst_ptr[i] = src_ptr[i];
    }

    return dst;
}

void *memset(void *dst, const int c, const usize count)
{
    u8 *dst_ptr = static_cast<u8 *>(dst);

    for (usize i = 0; i < count; i++) {
        dst_ptr[i] = static_cast<u8>(c);
    }

    return dst;
}

} // namespace kernel::memory
