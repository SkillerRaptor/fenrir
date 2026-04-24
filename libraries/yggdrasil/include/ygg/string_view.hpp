/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/string.hpp"
#include "ygg/types.hpp"

namespace ygg {
class StringView {
public:
    class Iterator {
    public:
        constexpr explicit Iterator(const char *ptr)
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
            const Iterator iterator(*this);
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
            const Iterator iterator(*this);
            --(*this);
            return iterator;
        }

        Iterator operator+(const usize offset) const { return Iterator(m_ptr + offset); }

        Iterator &operator+=(const usize offset)
        {
            m_ptr += offset;
            return *this;
        }

        Iterator operator-(const usize offset) const { return Iterator(m_ptr - offset); }

        Iterator &operator-=(const usize offset)
        {
            m_ptr -= offset;
            return *this;
        }

        constexpr const char *operator->() const { return m_ptr; }
        constexpr const char &operator*() const { return *m_ptr; }
        constexpr const char &operator[](const usize index) const { return m_ptr[index]; }

        constexpr bool operator==(const Iterator &other) const { return m_ptr == other.m_ptr; }
        constexpr bool operator!=(const Iterator &other) const { return m_ptr != other.m_ptr; }

    private:
        const char *m_ptr = nullptr;
    };

public:
    constexpr StringView() = default;

    constexpr StringView(const char *data)
        : m_data(data)
        , m_length(strlen(data))
    {
    }

    constexpr StringView(const char *data, const size_t length)
        : m_data(data)
        , m_length(length)
    {
    }

    constexpr char operator[](const usize index) const { return m_data[index]; }
    constexpr char front() const { return m_data[0]; }
    constexpr char back() const { return m_data[m_length - 1]; }
    constexpr const char *data() const noexcept { return m_data; }

    constexpr usize size() const { return m_length; }
    constexpr usize length() const { return m_length; }
    constexpr bool empty() const { return m_length == 0; }

    constexpr Iterator begin() const { return Iterator(m_data); }
    constexpr Iterator cbegin() const { return begin(); }

    constexpr Iterator end() const { return Iterator(m_data + m_length); }
    constexpr Iterator cend() const { return end(); }

private:
    const char *m_data = nullptr;
    usize m_length = 0;
};

} // namespace ygg
