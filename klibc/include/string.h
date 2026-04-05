/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// String examination
size_t strlen(const char *str);

// Character array manipulation
int memcmp(const void *lhs, const void *rhs, size_t n);
void *memset(void *dst, int c, size_t n);
void *memcpy(void *__restrict dst, const void *__restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);

#ifdef __cplusplus
}
#endif
