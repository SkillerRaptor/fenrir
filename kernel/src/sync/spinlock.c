/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "sync/spinlock.h"

#include "arch/x86_64/cpu.h"

void spinlock_lock(struct Spinlock *spinlock)
{
    while (__atomic_test_and_set(&spinlock->lock, __ATOMIC_ACQUIRE)) {
        cpu_pause();
    }
}

void spinlock_unlock(struct Spinlock *spinlock) { __atomic_clear(&spinlock->lock, __ATOMIC_RELEASE); }
