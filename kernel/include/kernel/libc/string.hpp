/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

extern "C" {

usize strlen(const char *str);

void *memcpy(void *dst, const void *src, usize count);

int memcmp(const void *lhs, const void *rhs, usize count);

void *memset(void *dst, int c, usize count);

void *memmove(void *dst, const void *src, usize count);

} // extern "C"
