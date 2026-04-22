/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/assert.hpp"

#include "acpi/apic.hpp"
#include "core/logger.hpp"
#include "lib/panic.hpp"

#ifdef __cplusplus
extern "C" {
#endif

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, const u32 line, const char *function)
{
    logger::fatal("\n");
    logger::fatal(
        "Assertion \033[38;2;0;0;255m{}\033[0m failed in \033[38;2;255;215;0m{}\033[0m at \033[38;2;0;128;0m{}:{}\n",
        assertion,
        function,
        file,
        line);

    panic();
}

#ifdef __cplusplus
}
#endif
