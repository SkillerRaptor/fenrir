/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace kernel::hpet {

void initialize();

void sleep(u64 ms);

} // namespace kernel::hpet
