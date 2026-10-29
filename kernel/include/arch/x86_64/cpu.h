/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

static inline void cpu_halt() { asm volatile("hlt"); }

static inline void cpu_pause() { asm volatile("pause"); }

static inline void cpu_enable_interrupts() { asm volatile("sti"); }

static inline void cpu_disable_interrupts() { asm volatile("cli"); }

[[noreturn]] static inline void cpu_hcf()
{
    while (true) {
        cpu_disable_interrupts();
        cpu_halt();
    }
}

static inline uint64_t cpu_flags()
{
    uint64_t flags = 0;
    asm volatile("pushfq\n"
                 "pop %0"
                 : "=r"(flags)
                 :
                 : "memory");
    return flags;
}

static inline bool cpu_are_interrupts_enabled() { return (cpu_flags() & (1 << 9)) != 0; }
