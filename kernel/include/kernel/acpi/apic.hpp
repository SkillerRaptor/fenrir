/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace kernel::apic {

void initialize();

void send_eoi();

} // namespace kernel::apic
