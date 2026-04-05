/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace lib::math {

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

} // namespace lib::math
