/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "sync/spinlock.h"

#include "arch/x86_64/cpu.h"
#include "lib/assertion.h"

void spinlock_lock(struct Spinlock *spinlock)
{
    VERIFY(!cpu_are_interrupts_enabled());

    while (__atomic_test_and_set(&spinlock->lock, __ATOMIC_ACQUIRE)) {
        cpu_pause();
    }
}

void spinlock_unlock(struct Spinlock *spinlock)
{
    VERIFY(!cpu_are_interrupts_enabled());

    __atomic_clear(&spinlock->lock, __ATOMIC_RELEASE);
}
