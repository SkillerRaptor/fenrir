/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/registers.hpp"

namespace idt {

using InterruptHandler = void (*)(const Registers &);

void initialize();
void load();

void set_handler(u8 isr, InterruptHandler);

} // namespace idt
