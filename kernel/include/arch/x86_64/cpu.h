/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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
