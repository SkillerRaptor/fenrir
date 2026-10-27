/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/assert.h"

#include "arch/x86_64/cpu.h"
#include "lib/log.h"

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, uint32_t line, const char *function)
{
    LOG_ERROR("\n");
    LOG_ERROR("Assertion '%s' failed in %s at %s:%u\n", assertion, function, file, line);
    LOG_ERROR("\n");

    cpu_hcf();
}
