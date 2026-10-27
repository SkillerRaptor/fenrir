/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

static inline uint64_t math_div_round_up(uint64_t value, uint64_t divisor)
{
    return ((value + (divisor - 1)) / divisor);
}

static inline uint64_t math_align_up(uint64_t value, uint64_t alignment)
{
    return math_div_round_up(value, alignment) * alignment;
}

static inline uint64_t math_align_down(uint64_t value, uint64_t alignment) { return ((value / alignment) * alignment); }
