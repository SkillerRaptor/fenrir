/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "assert.hpp"
#include "lib/types.hpp"

namespace lib {

template <typename T>
class Vector {
public:
    Vector() = default;
    ~Vector() { delete[] m_data; }

    void push_back(const T &value)
    {
        if (m_size >= m_capacity) {
            reallocate(m_capacity == 0 ? 1 : m_capacity * 2);
        }

        m_data[m_size] = value;
        ++m_size;
    }

    void pop_back()
    {
        assert(!is_empty());
        --m_size;
    }

    bool is_empty() const { return size() == 0; }

    T *data() const { return m_data; }
    usize size() const { return m_size; }
    usize capacity() const { return m_capacity; }

    T &operator[](const usize index)
    {
        assert(index < m_size);
        return m_data[index];
    }

    const T &operator[](const usize index) const
    {
        assert(index < m_size);
        return m_data[index];
    }

private:
    void reallocate(const usize new_capacity)
    {
        assert(new_capacity != 0);

        T *new_block = new T[new_capacity];

        if (new_capacity < m_size) {
            m_size = new_capacity;
        }

        for (usize i = 0; i < m_size; ++i) {
            new_block[i] = m_data[i];
        }

        delete[] m_data;

        m_data = new_block;
        m_capacity = new_capacity;
    }

private:
    T *m_data { nullptr };
    usize m_size { 0 };
    usize m_capacity { 0 };
};

} // namespace lib
