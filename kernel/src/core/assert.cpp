/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "core/assert.hpp"

#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "sync/spinlock.hpp"

namespace kernel {

static Spinlock s_lock = {};

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, const u32 line, const char *function)
{
    s_lock.lock();

    logger::err("");
    logger::err("Assertion '%s' failed in %s at %s:%u", assertion, function, file, line);
    logger::err("");
    stacktrace::print(50);
    logger::err("");

    s_lock.unlock();

    for (;;) {
        __asm__ __volatile__("cli");
        __asm__ __volatile__("hlt");
    }
}

} // namespace kernel
