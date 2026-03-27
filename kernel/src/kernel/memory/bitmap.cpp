/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/memory/bitmap.hpp"

namespace kernel {

void Bitmap::set(const usize index, const bool value)
{
    if (index >= m_size) {
        return;
    }

    const usize byte = index / 8;
    const usize bit = index % 8;

    if (value) {
        m_data[byte] |= (1 << bit);
    } else {
        m_data[byte] &= ~(1 << bit);
    }
}

bool Bitmap::get(const usize index) const
{
    if (index >= m_size) {
        return false;
    }

    const usize byte = index / 8;
    const usize bit = index % 8;

    return m_data[byte] & (1 << bit);
}

} // namespace kernel
