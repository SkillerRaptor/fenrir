/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/lib/identifier.hpp"

namespace kernel {

struct ProcessIdTag;
using ProcessId = Identifier<ProcessIdTag, i32>;

struct ThreadIdTag;
using ThreadId = Identifier<ThreadIdTag, i32>;

} // namespace kernel
