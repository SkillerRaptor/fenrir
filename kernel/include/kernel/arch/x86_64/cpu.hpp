/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

namespace kernel::cpu {

inline void halt() { asm volatile("hlt"); }

inline void pause() { asm volatile("pause"); }

inline void enable_interrupts() { asm volatile("sti"); }

inline void disable_interrupts() { asm volatile("cli"); }

inline void write_msr(const u32 msr, const u64 value)
{
    const u32 high = (value >> 32) & 0xffff;
    const u32 low = (value >> 0) & 0xffff;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr) : "memory");
}

inline u64 read_msr(const u32 msr)
{
    u32 high = 0;
    u32 low = 0;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return (static_cast<u64>(high) << 32) | static_cast<u64>(low);
}

} // namespace kernel::cpu
