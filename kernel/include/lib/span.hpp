/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

template <typename T>
class Span {
public:
    class Iterator {
    public:
        constexpr explicit Iterator(T *ptr)
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

        constexpr T *operator->() const { return m_ptr; }
        constexpr T &operator*() const { return *m_ptr; }
        constexpr T &operator[](const usize index) const { return m_ptr[index]; }

        constexpr bool operator==(const Iterator &other) { return m_ptr == other.m_ptr; }
        constexpr bool operator!=(const Iterator &other) { return m_ptr != other.m_ptr; }

    private:
        T *m_ptr { nullptr };
    };

public:
    constexpr Span() = default;

    constexpr Span(T *data, const usize size)
        : m_data(data)
        , m_size(size)
    {
    }

    constexpr Span(T *first, T *last)
        : m_data(first)
        , m_size(static_cast<usize>(last - first))
    {
    }

    template <usize N>
    constexpr Span(T (&arr)[N])
        : m_data(arr)
        , m_size(N)
    {
    }

    constexpr bool empty() const { return m_size == 0; }

    constexpr T *data() const { return m_data; }
    constexpr usize size() const { return m_size; }

    constexpr T &front() const { return m_data[0]; }
    constexpr T &back() const { return m_data[m_size - 1]; }
    constexpr T &operator[](usize index) const { return m_data[index]; }

    constexpr Iterator begin() const { return Iterator(m_data); }
    constexpr Iterator end() const { return Iterator(m_data + m_size); }

    constexpr Iterator cbegin() const { return begin(); }
    constexpr Iterator cend() const { return end(); }

private:
    T *m_data = nullptr;
    usize m_size = 0;
};
