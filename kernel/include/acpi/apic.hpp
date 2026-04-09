/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

namespace apic {

void initialize();
void enable_lapic();

void send_eoi();
void send_ipi(u32 lapic_id, u8 vector);

} // namespace apic
