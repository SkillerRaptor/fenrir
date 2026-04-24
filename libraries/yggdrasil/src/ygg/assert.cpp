/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "ygg/assert.hpp"

namespace ygg::detail {

[[noreturn]] void
    __assertion_failed(const StringView assertion, const StringView file, const u32 line, const StringView function)
{
    /*
    logger::fatal("\n");
    logger::fatal(
        "Assertion \033[38;2;0;0;255m{}\033[0m failed in \033[38;2;255;215;0m{}\033[0m at \033[38;2;0;128;0m{}:{}\n",
        assertion,
        function,
        file,
        line);

    panic();
    */
}

} // namespace ygg::detail
