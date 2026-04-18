/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/hash.hpp"
#include "lib/types.hpp"

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
    T m_value = 0;
};

template <typename U, typename T>
struct Hash<Identifier<U, T>> {
    usize operator()(const Identifier<U, T> &key) const { return Hash<u64> { }(key.get()); }
};
