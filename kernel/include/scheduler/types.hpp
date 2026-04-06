/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/identifier.hpp"

namespace kernel {

struct ProcessIdTag;
using ProcessId = lib::Identifier<ProcessIdTag, i32>;

struct ThreadIdTag;
using ThreadId = lib::Identifier<ThreadIdTag, i32>;

} // namespace kernel
