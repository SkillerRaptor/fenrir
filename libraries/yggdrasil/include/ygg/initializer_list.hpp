/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/types.hpp"

namespace std {

template <typename T>
class initializer_list {
private:
    constexpr initializer_list(const T *data, usize size)
        : m_data(data)
        , m_size(size)
    {
    }

public:
    constexpr initializer_list() = default;

    constexpr usize size() const { return m_size; }

    constexpr const T *begin() const { return m_data; }
    constexpr const T *end() const { return m_data + m_size; }

private:
    const T *m_data = nullptr;
    usize m_size = 0;
};

} // namespace std

namespace ygg {

template <typename T>
using InitializerList = std::initializer_list<T>;

} // namespace ygg
