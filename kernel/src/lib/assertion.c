/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/assertion.h"

#include <stdarg.h>

#include <__stdarg_va_list.h>

#include "arch/x86_64/cpu.h"
#include "lib/log.h"
#include "lib/stdio.h"
#include "lib/string.h"

[[noreturn]] void
    _assertion_failed(const char *assertion, const char *file, uint32_t line, const char *function, const char *message)
{
    LOG_ERROR("\n");
    LOG_ERROR("Assertion '%s' failed in %s at %s:%u", assertion, function, file, line);
    if (strlen(message) > 0) {
        printf(": %s\n", message);
    } else {
        printf("\n");
    }
    LOG_ERROR("\n");

    cpu_hcf();
}

[[noreturn]] void _panic(const char *message, const char *file, uint32_t line, const char *function)
{
    LOG_ERROR("\n");
    LOG_ERROR("Panic in %s at %s:%u: %s\n", function, file, line, message);
    LOG_ERROR("\n");

    cpu_hcf();
}
