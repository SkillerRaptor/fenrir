/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscall/syscalls.hpp"

namespace syscalls {

u64 sys$close_device(ygg::Span<const u64>)
{
    // Close (Ignore)
    return 0;
}

} // namespace syscalls
