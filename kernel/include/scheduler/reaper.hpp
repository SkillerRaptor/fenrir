/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "scheduler/thread.hpp"

namespace reaper {

[[noreturn]] void run();

void add_thread_to_reap(Thread *);

} // namespace reaper
