/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "drivers/serial.hpp"

#include "arch/x86_64/io.hpp"

namespace kernel::serial {

static constexpr u16 s_port_base = 0x3f8;

static constexpr u16 s_port_receive_buffer = s_port_base + 0;
static constexpr u16 s_port_transmit_buffer = s_port_base + 0;

static constexpr u16 s_port_interrupt_enable = s_port_base + 1;

static constexpr u16 s_port_divisor_low = s_port_base + 0;
static constexpr u16 s_port_divisor_high = s_port_base + 1;

static constexpr u16 s_port_fifo_control = s_port_base + 2;
static constexpr u16 s_port_line_control = s_port_base + 3;
static constexpr u16 s_port_modem_control = s_port_base + 4;
static constexpr u16 s_port_line_status = s_port_base + 5;

void initialize()
{
    // NOTE: Disable all interrupts
    io::out8(s_port_interrupt_enable, 0x00);

    // NOTE: Enable DLAB
    io::out8(s_port_line_control, 0x80);

    // NOTE: Set divisor to 3
    io::out8(s_port_divisor_low, 0x03);
    io::out8(s_port_divisor_high, 0x00);

    // NOTE: Set line and fifo modes
    io::out8(s_port_line_control, 0x03);
    io::out8(s_port_fifo_control, 0xc7);

    // NOTE: Check if serial is faulty
    io::out8(s_port_modem_control, 0x0b);
    io::out8(s_port_modem_control, 0x1e);
    io::out8(s_port_transmit_buffer, 0xae);
    if (io::in8(s_port_receive_buffer) != 0xae) {
        return;
    }

    // NOTE: Set serial in normal operation mode
    io::out8(s_port_modem_control, 0x0f);
}

static bool is_transmit_empty() { return io::in8(s_port_line_status) & 0x20; }

void write(const char c)
{
    while (!is_transmit_empty()) { }

    io::out8(s_port_transmit_buffer, c);
}

} // namespace kernel::serial
