/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdarg.h>

[[gnu::format(printf, 1, 2)]] int printf(const char *restrict fmt, ...);
int vprintf(const char *restrict fmt, va_list args);
