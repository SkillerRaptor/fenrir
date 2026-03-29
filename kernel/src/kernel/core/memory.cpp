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

void *memmove(void *dst, const void *src, const usize count)
{
    u8 *dst_ptr = static_cast<u8 *>(dst);
    const u8 *src_ptr = static_cast<const u8 *>(src);
    
    if (src > dst) {
        for (usize i = 0; i < count; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    } else if (src < dst) {
        for (usize i = count; i > 0; i--) {
            dst_ptr[i - 1] = src_ptr[i - 1];
        }
    }

    return dst;
}

int memcmp(const void *lhs, const void *rhs, const usize count)
{
    const u8 *lhs_ptr = static_cast<const u8 *>(lhs);
    const u8 *rhs_ptr = static_cast<const u8 *>(rhs);

    for (usize i = 0; i < count; i++) {
        const u8 lhs_value = lhs_ptr[i];
        const u8 rhs_value = rhs_ptr[i];
        if (lhs_value != rhs_value) {
            return lhs_value < rhs_value ? -1 : 1;
        }
    }

    return 0;
}

} // namespace kernel::memory
