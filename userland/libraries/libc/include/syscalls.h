/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

#define SYS_EXIT 0x01
#define SYS_WRITE 0x02

extern long syscall(long long number, ...);

void write(const char* str, size_t len);