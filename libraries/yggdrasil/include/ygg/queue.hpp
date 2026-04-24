/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/assert.hpp"
#include "ygg/types.hpp"
#include "ygg/vector.hpp"

namespace ygg {
template <typename T>
class Queue {
public:
    Queue() = default;
    ~Queue() = default;

    void push_back(const T &value) { m_data.push_back(value); }

    T pop_front()
    {
        ASSERT(!is_empty());

        const T front = m_data[0];

        for (usize i = 1; i < m_data.size(); ++i) {
            m_data[i - 1] = m_data[i];
        }

        m_data.pop_back();

        return front;
    }

    bool is_empty() const { return size() == 0; }

    usize size() const { return m_data.size(); }

private:
    Vector<T> m_data { };
};

} // namespace ygg
