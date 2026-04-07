/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdatomic.h>

class Spinlock {
public:
    void lock();
    void unlock();

private:
    atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

class SpinlockLocker {
public:
    explicit SpinlockLocker(Spinlock &lock)
        : m_lock(lock)
    {
        m_lock.lock();
    }

    ~SpinlockLocker() { m_lock.unlock(); }

    SpinlockLocker(const SpinlockLocker &) = delete;
    SpinlockLocker &operator=(const SpinlockLocker &) = delete;

    SpinlockLocker(SpinlockLocker &&) noexcept = delete;
    SpinlockLocker &operator=(SpinlockLocker &&) noexcept = delete;

private:
    Spinlock &m_lock;
};

template <typename T>
class SpinlockProtected {
public:
    // TODO: Add constructor to pass arguments
    SpinlockProtected() = default;
    ~SpinlockProtected() = default;

    SpinlockProtected(const SpinlockProtected &) = delete;
    SpinlockProtected &operator=(const SpinlockProtected &) = delete;

    SpinlockProtected(SpinlockProtected &&) noexcept = delete;
    SpinlockProtected &operator=(SpinlockProtected &&) noexcept = delete;

    template <typename Fn>
    auto with(const Fn &callback)
    {
        SpinlockLocker _locker(m_lock);
        return callback(m_value);
    }

private:
    T m_value {};
    Spinlock m_lock {};
};
