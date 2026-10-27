/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/registers.h"

typedef void (*InterruptHandler)(struct Registers *);

void idt_init();
void idt_register_handler(uint8_t isr, InterruptHandler interrupt_handler);
