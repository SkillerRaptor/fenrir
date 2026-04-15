/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/registers.hpp"
#include "lib/types.hpp"
#include "scheduler/types.hpp"

struct Process;

struct Thread {
    enum class State {
        Idle = 0,
        Busy,
        Dead,
    };

    ThreadId id { -1 };
    State state { State::Idle };

    Registers registers { };
    u8 *stack { nullptr };
    usize stack_size { 0 };

    Process *process { nullptr };
    Thread *next_thread { nullptr };
    Thread *thread_list { nullptr };
};
