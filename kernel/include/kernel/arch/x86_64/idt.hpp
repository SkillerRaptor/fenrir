/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "kernel/arch/x86_64/registers.hpp"

namespace kernel::idt {

using InterruptHandler = void (*)(const Registers &);

void initialize();

void set_handler(u8 isr, InterruptHandler);

} // namespace kernel::idt
