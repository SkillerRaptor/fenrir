/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace ustar {

// FIXME: Make this an impl class

struct File {
    const u8 *ptr = nullptr;
    usize size = 0;
};

File lookup(const u8 *archive, const char *file_name);

} // namespace ustar
