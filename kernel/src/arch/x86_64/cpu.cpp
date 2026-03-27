/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/cpu.hpp"

namespace cpu {

void halt() { asm volatile("hlt"); }

void enable_interrupts() { asm volatile("sti"); }

void disable_interrupts() { asm volatile("cli"); }

} // namespace cpu
