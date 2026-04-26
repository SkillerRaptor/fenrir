//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub fn div_round_up(x: u64, y: u64) -> u64 {
    (x + (y - 1)) / y
}

pub fn align_up(x: u64, y: u64) -> u64 {
    div_round_up(x, y) * y
}

pub fn align_down(x: u64, y: u64) -> u64 {
    (x / y) * y
}
