/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "klibc/assert.h"

#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "sync/spinlock.hpp"

#ifdef __cplusplus
extern "C" {
#endif

static kernel::Spinlock s_lock = { };

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, const u32 line, const char *function)
{
    s_lock.lock();

    kernel::logger::err("");
    kernel::logger::err("Assertion '%s' failed in %s at %s:%u", assertion, function, file, line);
    kernel::logger::err("");
    kernel::stacktrace::print(50);
    kernel::logger::err("");

    s_lock.unlock();

    for (;;) {
        __asm__ __volatile__("cli");
        __asm__ __volatile__("hlt");
    }
}

#ifdef __cplusplus
}
#endif
