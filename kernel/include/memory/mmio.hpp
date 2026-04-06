/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace kernel::mmio {

template <typename T>
T in(const u64 addr)
{
    switch (sizeof(T)) {
    case sizeof(u8): {
        volatile u8 *ptr = reinterpret_cast<volatile u8 *>(addr);
        return *ptr;
    }
    case sizeof(u16): {
        volatile u16 *ptr = reinterpret_cast<volatile u16 *>(addr);
        return *ptr;
    }
    case sizeof(u32): {
        volatile u32 *ptr = reinterpret_cast<volatile u32 *>(addr);
        return *ptr;
    }
    case sizeof(u64): {
        volatile u64 *ptr = reinterpret_cast<volatile u64 *>(addr);
        return *ptr;
    }
    default:
        assert(false);
    }
}

template <typename T>
void out(const u64 addr, const T value)
{
    switch (sizeof(T)) {
    case sizeof(u8): {
        volatile u8 *ptr = reinterpret_cast<volatile u8 *>(addr);
        *ptr = value;
        break;
    }
    case sizeof(u16): {
        volatile u16 *ptr = reinterpret_cast<volatile u16 *>(addr);
        *ptr = value;
        break;
    }
    case sizeof(u32): {
        volatile u32 *ptr = reinterpret_cast<volatile u32 *>(addr);
        *ptr = value;
        break;
    }
    case sizeof(u64): {
        volatile u64 *ptr = reinterpret_cast<volatile u64 *>(addr);
        *ptr = value;
        break;
    }
    default:
        assert(false);
    }
}

} // namespace kernel::mmio
