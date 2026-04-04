/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/arch/x86_64/registers.hpp"
#include "kernel/core/types.hpp"
#include "kernel/scheduler/process.hpp"

namespace kernel {

struct Thread {
    using Id = i32;

    enum class State {
        Idle = 0,
        Busy,
        Dead,
    };

    Process::Id pid { -1 };
    Id tid { -1 };
    State state { State::Idle };

    Registers registers { };
    u8 *stack { nullptr };
    usize stack_size { 0 };
};

} // namespace kernel
