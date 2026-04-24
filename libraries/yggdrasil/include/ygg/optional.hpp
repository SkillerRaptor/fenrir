/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/assert.hpp"
#include "ygg/types.hpp"

namespace ygg {
struct NoneType {
    constexpr NoneType() = default;
};

static constexpr NoneType None { };

template <typename T>
struct SomeType {
    T value;
};

template <typename T>
SomeType<T> Some(T value)
{
    return SomeType<T> { value };
}

template <typename T>
class Optional {
public:
    constexpr Optional()
        : m_empty()
    {
    }

    constexpr Optional(NoneType)
        : m_empty()
    {
    }

    constexpr Optional(const SomeType<T> &value)
        : m_value(value.value)
        , m_has_value(true)
    {
    }

    constexpr Optional(SomeType<T> &&value)
        : m_value(static_cast<T &&>(value.value))
        , m_has_value(true)
    {
    }

    Optional(const Optional &other)
        : m_empty()
        , m_has_value(other.m_has_value)
    {
        if (m_has_value) {
            m_value = other.m_value;
        }
    }

    Optional(Optional &&other) noexcept
        : m_empty()
        , m_has_value(other.m_has_value)
    {
        if (m_has_value) {
            m_value = static_cast<T &&>(other.m_value);
            other.m_has_value = false;
        }
    }

    ~Optional()
    {
        if (m_has_value) {
            m_value.~T();
        }
    }

    Optional &operator=(NoneType)
    {
        reset();
        return *this;
    }

    Optional &operator=(const SomeType<T> &value)
    {
        m_value = value.value;
        m_has_value = true;
        return *this;
    }

    Optional &operator=(SomeType<T> &&value)
    {
        m_value = static_cast<T &&>(value.value);
        m_has_value = true;

        return *this;
    }

    Optional &operator=(const Optional &other)
    {
        if (this == &other) {
            return *this;
        }

        if (!other.m_has_value) {
            reset();
            return *this;
        }

        m_value = other.m_value;
        m_has_value = true;

        return *this;
    }

    Optional &operator=(Optional &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        if (!other.m_has_value) {
            return *this;
        }

        m_value = static_cast<T &&>(other.m_value);
        m_has_value = true;
        other.m_has_value = false;

        return *this;
    }

    T &unwrap()
    {
        ASSERT(m_has_value);
        return m_value;
    }

    const T &unwrap() const
    {
        ASSERT(m_has_value);
        return m_value;
    }

    T unwrap_or(const T &default_value) const { return m_has_value ? m_value : default_value; }

    const NoneType &unwrap_other() const
    {
        ASSERT(!m_has_value);
        return None;
    }

    constexpr bool is_some() const { return m_has_value; }
    constexpr bool is_none() const { return !m_has_value; }

    constexpr explicit operator bool() const { return m_has_value; }

private:
    void reset()
    {
        if (!m_has_value) {
            return;
        }

        m_value.~T();
        m_has_value = false;
    }

private:
    union {
        u8 m_empty = 0;
        T m_value;
    };

    bool m_has_value = false;
};

template <typename T>
class Optional<T *> {
public:
    constexpr Optional() = default;

    constexpr Optional(NoneType) { }

    constexpr Optional(SomeType<T *> ptr)
        : m_ptr(ptr.value)
    {
    }

    Optional &operator=(NoneType)
    {
        m_ptr = nullptr;
        return *this;
    }

    Optional &operator=(SomeType<T *> ptr)
    {
        m_ptr = ptr.value;
        return *this;
    }

    T *unwrap() const
    {
        ASSERT(m_ptr != nullptr);
        return m_ptr;
    }

    T *unwrap_or(T *default_value) const { return m_ptr ? m_ptr : default_value; }

    constexpr bool has_value() const { return m_ptr != nullptr; }
    constexpr explicit operator bool() const { return has_value(); }

private:
    void reset() { m_ptr = nullptr; }

private:
    T *m_ptr = nullptr;
};

} // namespace ygg
