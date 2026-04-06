/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace apic {

void initialize();
void enable_lapic();

void send_eoi();

} // namespace apic
