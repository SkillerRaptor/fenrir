/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace kernel {

namespace cpu {

struct Info;

} // namespace cpu

namespace smp {

void initialize();

cpu::Info *get_cpu_infos();

} // namespace smp

} // namespace kernel
