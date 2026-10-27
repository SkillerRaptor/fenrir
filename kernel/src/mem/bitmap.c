/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "mem/bitmap.h"

#include "lib/assert.h"
#include "lib/string.h"

void bitmap_set(struct Bitmap *bitmap, size_t index, bool value)
{
    ASSERT(index < bitmap->size * 8);

    const size_t byte_index = index / 8;
    const size_t bit_index = index % 8;

    uint8_t byte_value = bitmap->data[byte_index];

    if (value) {
        byte_value |= 1 << bit_index;
    } else {
        byte_value &= ~(1 << bit_index);
    }

    bitmap->data[byte_index] = byte_value;
}

bool bitmap_is_set(const struct Bitmap *bitmap, size_t index)
{
    ASSERT(index < bitmap->size * 8);

    const size_t byte_index = index / 8;
    const size_t bit_index = index % 8;

    const uint8_t byte_value = bitmap->data[byte_index];
    const uint8_t bit_value = byte_value & (1 << bit_index);

    return bit_value != 0;
}

void bitmap_fill(struct Bitmap *bitmap, uint8_t value) { memset(bitmap->data, value, bitmap->size); }
