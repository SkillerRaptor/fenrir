//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use crate::arch::x86_64::io;

const PORT_BASE: u16 = 0x3f8;

const PORT_RECEIVE_BUFFER: u16 = PORT_BASE + 0;
const PORT_TRANSMIT_BUFFER: u16 = PORT_BASE + 0;

const PORT_INTERRUPT_ENABLE: u16 = PORT_BASE + 1;

const PORT_DIVISOR_LOW: u16 = PORT_BASE + 0;
const PORT_DIVISOR_HIGH: u16 = PORT_BASE + 1;

const PORT_FIFO_CONTROL: u16 = PORT_BASE + 2;
const PORT_LINE_CONTROL: u16 = PORT_BASE + 3;
const PORT_MODEM_CONTROL: u16 = PORT_BASE + 4;
const PORT_LINE_STATUS: u16 = PORT_BASE + 5;

pub fn initialize() {
    // NOTE: Disable all interrupts
    io::out8(PORT_INTERRUPT_ENABLE, 0x00);

    // NOTE: Enable DLAB
    io::out8(PORT_LINE_CONTROL, 0x80);

    // NOTE: Set divisor to 3
    io::out8(PORT_DIVISOR_LOW, 0x03);
    io::out8(PORT_DIVISOR_HIGH, 0x00);

    // NOTE: Set line and fifo modes
    io::out8(PORT_LINE_CONTROL, 0x03);
    io::out8(PORT_FIFO_CONTROL, 0xc7);

    // NOTE: Check if serial is faulty
    io::out8(PORT_MODEM_CONTROL, 0x0b);
    io::out8(PORT_MODEM_CONTROL, 0x1e);
    io::out8(PORT_TRANSMIT_BUFFER, 0xae);
    if io::in8(PORT_RECEIVE_BUFFER) != 0xae {
        return;
    }

    // NOTE: Set serial in normal operation mode
    io::out8(PORT_MODEM_CONTROL, 0x0f);
}

fn is_transmit_empty() -> bool {
    (io::in8(PORT_LINE_STATUS) & 0x20) != 0
}

pub fn write(c: char) {
    while !is_transmit_empty() {}

    io::out8(PORT_TRANSMIT_BUFFER, c as u8);
}
