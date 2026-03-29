/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdatomic.h>

namespace kernel {

class Spinlock {
public:
    void lock();
    void unlock();

private:
    atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

} // namespace kernel
