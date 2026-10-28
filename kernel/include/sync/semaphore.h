/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "sync/spinlock.h"

struct Semaphore {
    struct Spinlock lock;
    uint64_t counter;
};
