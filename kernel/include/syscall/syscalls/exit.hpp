/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

struct SyscallRegisters;

namespace syscalls {

void sys$exit(const SyscallRegisters *);

} // namespace syscalls
