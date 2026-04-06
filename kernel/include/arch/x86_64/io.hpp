/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace kernel::io {

inline u8 in8(const u16 port)
{
    volatile u8 value = 0;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline void out8(const u16 port, const u8 value) { asm volatile("outb %0, %1" : : "a"(value), "Nd"(port)); }

inline u16 in16(const u16 port)
{
    volatile u16 value = 0;
    asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline void out16(const u16 port, const u16 value) { asm volatile("outw %0, %1" : : "a"(value), "Nd"(port)); }

inline u32 in32(const u16 port)
{
    volatile u32 value = 0;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline void out32(const u16 port, const u32 value) { asm volatile("outl %0, %1" : : "a"(value), "Nd"(port)); }

inline void wait() { out8(0x80, 0x00); }

} // namespace kernel::io
