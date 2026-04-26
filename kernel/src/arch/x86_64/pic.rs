//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use crate::arch::x86_64::io;

const MASTER_COMMAND_SELECTOR: u16 = 0x20;
const MASTER_DATA_SELECTOR: u16 = 0x21;

const SLAVE_COMMAND_SELECTOR: u16 = 0xa0;
const SLAVE_DATA_SELECTOR: u16 = 0xa1;

const ICW1_INIT: u8 = 1 << 4;
const ICW1_ICW4: u8 = 1 << 0;

const MASTER_OFFSET: u8 = 0x20;
const SLAVE_OFFSET: u8 = 0x28;

const MASTER_IDENTITY: u8 = 0x04;
const SLAVE_IDENTITY: u8 = 0x02;

const ICW1_8086: u8 = 1 << 0;

const DISABLE_MASK: u8 = 0xff;

pub fn disable() {
    io::out8(MASTER_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);
    io::out8(SLAVE_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);

    io::out8(MASTER_DATA_SELECTOR, MASTER_OFFSET);
    io::out8(SLAVE_DATA_SELECTOR, SLAVE_OFFSET);

    io::out8(MASTER_DATA_SELECTOR, MASTER_IDENTITY);
    io::out8(SLAVE_DATA_SELECTOR, SLAVE_IDENTITY);

    io::out8(MASTER_DATA_SELECTOR, ICW1_8086);
    io::out8(SLAVE_DATA_SELECTOR, ICW1_8086);

    io::out8(MASTER_DATA_SELECTOR, DISABLE_MASK);
    io::out8(SLAVE_DATA_SELECTOR, DISABLE_MASK);

    log::info!("PIC: Disabled");
}
