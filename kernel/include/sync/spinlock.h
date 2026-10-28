/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

struct Spinlock {
    volatile uint32_t lock;
};

void spinlock_lock(struct Spinlock *spinlock);
void spinlock_unlock(struct Spinlock *spinlock);
