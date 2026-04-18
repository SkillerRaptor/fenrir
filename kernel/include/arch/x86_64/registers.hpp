/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

struct Registers {
    u64 r15 = 0;
    u64 r14 = 0;
    u64 r13 = 0;
    u64 r12 = 0;
    u64 r11 = 0;
    u64 r10 = 0;
    u64 r9 = 0;
    u64 r8 = 0;

    u64 rsi = 0;
    u64 rdi = 0;
    u64 rbp = 0;
    u64 rdx = 0;
    u64 rcx = 0;
    u64 rbx = 0;
    u64 rax = 0;

    u64 isr = 0;
    u64 error = 0;

    u64 rip = 0;
    u64 cs = 0;
    u64 flags = 0;
    u64 rsp = 0;
    u64 ss = 0;
};
