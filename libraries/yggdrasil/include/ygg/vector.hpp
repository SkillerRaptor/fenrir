/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/assert.hpp"
#include "ygg/types.hpp"

namespace ygg {
template <typename T>
class Vector {
public:
    class Iterator {
    public:
        explicit Iterator(T *ptr)
            : m_ptr(ptr)
        {
        }

        Iterator &operator++()
        {
            ++m_ptr;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator iterator(*this);
            ++(*this);
            return iterator;
        }

        Iterator &operator--()
        {
            --m_ptr;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator iterator(*this);
            --(*this);
            return iterator;
        }

        T &operator[](const usize index) const { return *(m_ptr + index); }

        T *operator->() { return m_ptr; }
        T &operator*() { return *m_ptr; }

        bool operator==(const Iterator &other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const Iterator &other) const { return m_ptr != other.m_ptr; }

    private:
        T *m_ptr = nullptr;
    };

    class ConstIterator {
    public:
        explicit ConstIterator(const T *ptr)
            : m_ptr(ptr)
        {
        }

        ConstIterator &operator++()
        {
            ++m_ptr;
            return *this;
        }

        ConstIterator operator++(int)
        {
            Iterator iterator(*this);
            ++(*this);
            return iterator;
        }

        ConstIterator &operator--()
        {
            --m_ptr;
            return *this;
        }

        ConstIterator operator--(int)
        {
            ConstIterator iterator(*this);
            --(*this);
            return iterator;
        }

        const T &operator[](const usize index) const { return *(m_ptr + index); }

        const T *operator->() { return m_ptr; }
        const T &operator*() { return *m_ptr; }

        bool operator==(const ConstIterator &other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const ConstIterator &other) const { return m_ptr != other.m_ptr; }

    private:
        const T *m_ptr = nullptr;
    };

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
        ASSERT(!is_empty());
        --m_size;
    }

    bool is_empty() const { return size() == 0; }

    T *data() const { return m_data; }
    usize size() const { return m_size; }
    usize capacity() const { return m_capacity; }

    T &operator[](const usize index)
    {
        ASSERT(index < m_size);
        return m_data[index];
    }

    const T &operator[](const usize index) const
    {
        ASSERT(index < m_size);
        return m_data[index];
    }

    Iterator begin() { return Iterator(m_data); }
    Iterator end() { return Iterator(m_data + m_size); }

    ConstIterator begin() const { return ConstIterator(m_data); }
    ConstIterator end() const { return ConstIterator(m_data + m_size); }

private:
    void reallocate(const usize new_capacity)
    {
        ASSERT(new_capacity != 0);

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
    T *m_data = nullptr;
    usize m_size = 0;
    usize m_capacity = 0;
};

} // namespace ygg
