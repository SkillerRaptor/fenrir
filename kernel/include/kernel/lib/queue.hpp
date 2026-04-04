/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"
#include "vector.hpp"

namespace kernel {

template <typename T>
class Queue {
public:
    Queue() = default;
    ~Queue() = default;

    void push_back(const T &value)
    {
        if (m_size >= m_data.capacity()) {
            grow();
        }

        const usize index = (m_head + m_size) % m_data.capacity();
        m_data[index] = value;
        ++m_size;
    }

    T pop_front()
    {
        // TODO: Panic if empty
        T value = m_data[m_head];
        m_head = (m_head + 1) % m_data.capacity();
        --m_size;
        return value;
    }

    bool is_empty() const { return size() == 0; }

    usize size() const { return m_size; }

private:
    void grow()
    {
        const usize old_capacity = m_data.capacity();
        const usize new_capacity = old_capacity + old_capacity / 2;

        Vector<T> new_data { };
        for (usize i = 0; i < new_capacity; ++i) {
            new_data.push_back(T { });
        }

        for (usize i = 0; i < m_size; ++i) {
            new_data[i] = m_data[(m_head + i) % old_capacity];
        }

        m_data = new_data;
        m_head = 0;
    }

private:
    Vector<T> m_data { };
    usize m_head { 0 };
    usize m_size { 0 };
};

} // namespace kernel
