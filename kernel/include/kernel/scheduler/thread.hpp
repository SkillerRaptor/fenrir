/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lib/types.hpp>

#include "kernel/arch/x86_64/registers.hpp"
#include "kernel/scheduler/types.hpp"

namespace kernel {

struct Thread {
    enum class State {
        Idle = 0,
        Busy,
        Dead,
    };

    ProcessId pid { -1 };
    ThreadId tid { -1 };
    State state { State::Idle };

    Registers registers {};
    u8 *stack { nullptr };
    usize stack_size { 0 };
};

} // namespace kernel
