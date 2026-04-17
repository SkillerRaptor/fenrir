/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscalls.h"

void write(const char *str, const size_t len) { syscall(SYS_WRITE, str, len); }
