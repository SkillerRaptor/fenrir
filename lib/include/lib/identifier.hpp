/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace lib {

template <typename, typename T = u32>
class Identifier {
public:
    constexpr Identifier() = default;

    constexpr explicit Identifier(T value)
        : m_value(value)
    {
    }

    constexpr bool operator==(const Identifier &other) const { return m_value == other.m_value; }
    constexpr bool operator!=(const Identifier &other) const { return m_value != other.m_value; }
    constexpr bool operator<(const Identifier &other) const { return m_value < other.m_value; }
    constexpr bool operator<=(const Identifier &other) const { return m_value <= other.m_value; }
    constexpr bool operator>(const Identifier &other) const { return m_value > other.m_value; }
    constexpr bool operator>=(const Identifier &other) const { return m_value >= other.m_value; }

    constexpr explicit operator T() const { return m_value; }

    constexpr T get() const { return m_value; }

private:
    T m_value { 0 };
};

} // namespace lib
