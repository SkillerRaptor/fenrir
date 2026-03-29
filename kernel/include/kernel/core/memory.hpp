/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

namespace kernel::memory {

static constexpr usize s_page_size { 4096 };

extern "C" void *memcpy(void *dst, const void *src, usize count);
extern "C" void *memset(void *dst, int c, usize count);
extern "C" void *memmove(void *dst, const void *src, usize count);
extern "C" int memcmp(const void *lhs, const void *rhs, usize count);

template <typename T>
constexpr T div_round_up(const T x, const T y)
{
    return (x + (y - 1)) / y;
}

template <typename T>
constexpr T align_up(const T x, const T y)
{
    return div_round_up(x, y) * y;
}

template <typename T>
constexpr T align_down(const T x, const T y)
{
    return (x / y) * y;
}

} // namespace kernel::memory
