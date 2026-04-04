/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/core/types.hpp"
#include "kernel/memory/vmm.hpp"

namespace kernel {

struct Process {
    using Id = i32;

    enum class State {
        Idle = 0,
        Busy,
        Dead,
    };

    Id pid { -1 };
    State state { State::Idle };

    vmm::PageMap *page_map { nullptr };
};

} // namespace kernel
