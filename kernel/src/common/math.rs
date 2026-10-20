//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub fn align_up(x: u64, y: u64) -> u64 {
    x.div_ceil(y) * y
}

pub fn align_down(x: u64, y: u64) -> u64 {
    (x / y) * y
}
