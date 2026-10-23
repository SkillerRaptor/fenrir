/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/stdio.h"

#include "lib/console.h"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>

#include <nanoprintf.h>

static void internal_putc(int c, void *) { console_putc(c); }

int printf(const char *restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);

    return ret;
}

int vprintf(const char *restrict fmt, va_list args)
{
    int ret = npf_vpprintf(internal_putc, NULL, fmt, args);
    return ret;
}
