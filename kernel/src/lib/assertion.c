/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/assertion.h"

#include <stdarg.h>

#include "arch/x86_64/cpu.h"
#include "lib/log.h"
#include "lib/stacktrace.h"
#include "lib/stdio.h"
#include "lib/string.h"

static constexpr char COLOR_RESET[] = "\x1b[0m";
static constexpr char COLOR_ASSERTION[] = "\x1b[38;2;50;50;255m";
static constexpr char COLOR_FUNCTION[] = "\x1b[38;2;255;215;0m";
static constexpr char COLOR_FILE[] = "\x1b[38;2;0;128;0m";

[[noreturn]] void
    _assertion_failed(const char *assertion, const char *file, uint32_t line, const char *function, const char *message)
{
    LOG_ERROR("\n");
    LOG_ERROR(
        "Assertion '%s%s%s' failed in %s%s%s at %s%s:%u%s",
        COLOR_ASSERTION,
        assertion,
        COLOR_RESET,
        COLOR_FUNCTION,
        function,
        COLOR_RESET,
        COLOR_FILE,
        file,
        line,
        COLOR_RESET);
    if (strlen(message) > 0) {
        printf(": %s\n", message);
    } else {
        printf("\n");
    }
    LOG_ERROR("\n");

    LOG_ERROR("Stacktrace:\n");
    stacktrace_print(50);

    LOG_ERROR("\n");

    cpu_hcf();
}

[[noreturn]] void _panic(const char *message, const char *file, uint32_t line, const char *function)
{
    LOG_ERROR("\n");
    LOG_ERROR(
        "Panic in %s%s%s at %s%s:%u%s: %s\n",
        COLOR_FUNCTION,
        function,
        COLOR_RESET,
        COLOR_FILE,
        file,
        line,
        COLOR_RESET,
        message);
    LOG_ERROR("\n");

    LOG_ERROR("Stacktrace:\n");
    stacktrace_print(50);

    LOG_ERROR("\n");

    cpu_hcf();
}
