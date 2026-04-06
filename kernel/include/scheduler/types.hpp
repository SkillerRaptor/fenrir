/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/identifier.hpp"

struct ProcessIdTag;
using ProcessId = Identifier<ProcessIdTag, i32>;

struct ThreadIdTag;
using ThreadId = Identifier<ThreadIdTag, i32>;
