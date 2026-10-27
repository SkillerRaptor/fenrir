/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.h"

void cpu_halt() { asm volatile("hlt"); }

void cpu_pause() { asm volatile("pause"); }

void cpu_enable_interrupts() { asm volatile("sti"); }

void cpu_disable_interrupts() { asm volatile("cli"); }

void cpu_hcf()
{
    while (true) {
        cpu_disable_interrupts();
        cpu_halt();
    }
}
