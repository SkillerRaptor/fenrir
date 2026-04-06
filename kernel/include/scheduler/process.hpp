/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "memory/vmm.hpp"
#include "scheduler/types.hpp"

struct Process {
    enum class State {
        Idle = 0,
        Busy,
        Dead,
    };

    ProcessId pid { -1 };
    State state { State::Idle };

    vmm::PageMap *page_map { nullptr };
};
