/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

enum class MemoryOrder : u8 {
    Relaxed = __ATOMIC_RELAXED,
    Consume = __ATOMIC_CONSUME,
    Acquire = __ATOMIC_ACQUIRE,
    Release = __ATOMIC_RELEASE,
    AcqRel = __ATOMIC_ACQ_REL,
    SeqCst = __ATOMIC_SEQ_CST
};

template <typename T>
class Atomic {
public:
    Atomic() = default;

    Atomic(const T value)
        : m_value(value)
    {
    }

    Atomic(const Atomic &) = delete;
    Atomic &operator=(const Atomic &) = delete;

    Atomic(Atomic &&) noexcept = delete;
    Atomic &operator=(Atomic &&) noexcept = delete;

    void store(T value, const MemoryOrder memory_order = MemoryOrder::SeqCst)
    {
        __atomic_store(&m_value, &value, static_cast<u8>(memory_order));
    }

    T load(const MemoryOrder memory_order = MemoryOrder::SeqCst) const
    {
        return __atomic_load_n(&m_value, static_cast<u8>(memory_order));
    }

    T fetch_add(const T value, const MemoryOrder memory_order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_add(&m_value, value, static_cast<u8>(memory_order));
    }

    T fetch_sub(const T value, const MemoryOrder memory_order = MemoryOrder::SeqCst)
    {
        return __atomic_fetch_sub(&m_value, value, static_cast<u8>(memory_order));
    }

private:
    T m_value = 0;
};
