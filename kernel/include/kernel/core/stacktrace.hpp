/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"

namespace kernel::stacktrace {

void initialize();
void print(u64 max_frames);

} // namespace kernel::stacktrace
