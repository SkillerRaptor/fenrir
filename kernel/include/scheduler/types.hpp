/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <ygg/identifier.hpp>

struct ProcessIdTag;
using ProcessId = ygg::Identifier<ProcessIdTag, i32>;

struct ThreadIdTag;
using ThreadId = ygg::Identifier<ThreadIdTag, i32>;
