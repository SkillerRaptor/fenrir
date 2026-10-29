/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/serial.h"

#include "arch/x86_64/io.h"

// TODO: Improve this by not using magic values

static constexpr uint16_t PORT_BASE = 0x3f8;
static constexpr uint16_t BUFFER_PORT = PORT_BASE + 0;

void serial_init()
{
    static constexpr uint16_t INTERRUPT_PORT = PORT_BASE + 1;

    static constexpr uint16_t DIVISOR_LOW_PORT = PORT_BASE + 0;
    static constexpr uint16_t DIVISOR_HIGH_PORT = PORT_BASE + 1;

    static constexpr uint16_t FIFO_CONTROL_PORT = PORT_BASE + 2;
    static constexpr uint16_t LINE_CONTROL_PORT = PORT_BASE + 3;
    static constexpr uint16_t MODEM_CONTROL_PORT = PORT_BASE + 4;

    io_out8(INTERRUPT_PORT, 0x00);

    io_out8(LINE_CONTROL_PORT, 0x80);

    io_out8(DIVISOR_LOW_PORT, 0x03);
    io_out8(DIVISOR_HIGH_PORT, 0x00);

    io_out8(LINE_CONTROL_PORT, 0x03);
    io_out8(FIFO_CONTROL_PORT, 0xc7);

    io_out8(MODEM_CONTROL_PORT, 0x0b);
    io_out8(MODEM_CONTROL_PORT, 0x1e);

    io_out8(BUFFER_PORT, 0xae);
    if (io_in8(BUFFER_PORT) != 0xae) {
        return;
    }

    io_out8(MODEM_CONTROL_PORT, 0x0f);
}

static bool is_transmit_empty()
{
    static constexpr uint16_t LINE_STATUS_PORT = PORT_BASE + 5;
    return (io_in8(LINE_STATUS_PORT) & 0x20) != 0;
}

void serial_putc(int c)
{
    while (!is_transmit_empty()) { }

    io_out8(BUFFER_PORT, (uint8_t) c);
}
