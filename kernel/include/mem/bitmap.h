/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

struct Bitmap {
    uint8_t *data;
    size_t size;
};

void bitmap_set(struct Bitmap *bitmap, size_t index, bool value);
bool bitmap_is_set(const struct Bitmap *bitmap, size_t index);

void bitmap_fill(struct Bitmap *bitmap, uint8_t value);
