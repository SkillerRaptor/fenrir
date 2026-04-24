/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "ygg/bitmap.hpp"

#include "ygg/assert.hpp"

namespace ygg {

void Bitmap::set(const usize index, const bool value)
{
    ASSERT(index < m_size);

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
    ASSERT(index < m_size);

    const usize byte = index / 8;
    const usize bit = index % 8;

    return m_data[byte] & (1 << bit);
}

} // namespace ygg
