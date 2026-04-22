/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/assert.hpp"

template <typename T>
struct OkType {
    T value;
};

template <typename T>
OkType<T> Ok(T value)
{
    return OkType<T> { value };
}

template <typename T>
struct ErrorType {
    T value;
};

template <typename T>
ErrorType<T> Err(T value)
{
    return ErrorType<T> { value };
}

template <typename T, typename E>
class Result {
public:
    constexpr Result(const OkType<T> &value)
        : m_ok(value.value)
        , m_is_ok(true)
    {
    }

    constexpr Result(OkType<T> &&value)
        : m_ok(static_cast<T &&>(value.value))
        , m_is_ok(true)
    {
    }

    constexpr Result(const ErrorType<E> &value)
        : m_err(value.value)
        , m_is_ok(false)
    {
    }

    constexpr Result(ErrorType<E> &&value)
        : m_err(static_cast<E &&>(value.value))
        , m_is_ok(false)
    {
    }

    ~Result()
    {
        if (m_is_ok) {
            m_ok.~T();
        } else {
            m_err.~E();
        }
    }

    Result(const Result &other)
        : m_empty(0)
        , m_is_ok(other.m_is_ok)
    {
        if (m_is_ok) {
            m_ok = other.m_ok;
        } else {
            m_err = other.m_err;
        }
    }

    Result &operator=(const Result &other)
    {
        if (this == &other) {
            return *this;
        }

        if (m_is_ok) {
            m_ok.~T();
        } else {
            m_err.~E();
        }

        m_is_ok = other.m_is_ok;
        if (m_is_ok) {
            m_ok = other.m_ok;
        } else {
            m_err = other.m_err;
        }

        return *this;
    }

    Result(Result &&other) noexcept
        : m_empty(0)
        , m_is_ok(other.m_is_ok)
    {
        if (m_is_ok) {
            m_ok = static_cast<T &&>(other.m_ok);
        } else {
            m_err = static_cast<E &&>(other.m_err);
        }
    }

    Result &operator=(Result &&other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        if (m_is_ok) {
            m_ok.~T();
        } else {
            m_err.~E();
        }

        m_is_ok = other.m_is_ok;
        if (m_is_ok) {
            m_ok = static_cast<T &&>(other.m_ok);
        } else {
            m_err = static_cast<E &&>(other.m_err);
        }

        return *this;
    }

    T &unwrap()
    {
        assert(m_is_ok);
        return m_ok;
    }

    const T &unwrap() const
    {
        assert(m_is_ok);
        return m_ok;
    }

    T unwrap_or(const T &default_value) const { return m_is_ok ? m_ok : default_value; }

    E &unwrap_err()
    {
        assert(!m_is_ok);
        return m_err;
    }

    const E &unwrap_err() const
    {
        assert(!m_is_ok);
        return m_err;
    }

    ErrorType<E> unwrap_other() const
    {
        assert(!m_is_ok);
        return Err(m_err);
    }

    constexpr bool is_ok() const { return m_is_ok; }
    constexpr bool is_err() const { return !m_is_ok; }

    constexpr explicit operator bool() const { return m_is_ok; }

private:
    union {
        u8 m_empty = 0;
        T m_ok;
        E m_err;
    };

    bool m_is_ok = false;
};
