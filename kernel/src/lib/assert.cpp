/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/assert.hpp"

#include "acpi/apic.hpp"
#include "arch/x86_64/cpu.hpp"
#include "core/logger.hpp"
#include "core/stacktrace.hpp"

#ifdef __cplusplus
extern "C" {
#endif

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, const u32 line, const char *function)
{
    apic::send_ipi(0xff, 0xfe);

    logger::err("\n");
    logger::err("Assertion '%s' failed in %s(...) at %s:%u\n", assertion, function, file, line);
    logger::err("\n");
    stacktrace::print(50);
    logger::err("\n");

    Cpu::hcf();
}

#ifdef __cplusplus
}
#endif
