/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace cpu {

inline void halt() { asm volatile("hlt"); }

inline void enable_interrupts() { asm volatile("sti"); }

inline void disable_interrupts() { asm volatile("cli"); }

} // namespace cpu
