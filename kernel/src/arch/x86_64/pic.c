/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "arch/x86_64/pic.h"

#include "arch/x86_64/io.h"
#include "lib/log.h"

static constexpr uint16_t MASTER_COMMAND_SELECTOR = 0x20;
static constexpr uint16_t MASTER_DATA_SELECTOR = 0x21;

static constexpr uint16_t SLAVE_COMMAND_SELECTOR = 0xa0;
static constexpr uint16_t SLAVE_DATA_SELECTOR = 0xa1;

static constexpr uint8_t ICW1_INIT = 1 << 4;
static constexpr uint8_t ICW1_ICW4 = 1 << 0;

static constexpr uint8_t MASTER_OFFSET = 0x20;
static constexpr uint8_t SLAVE_OFFSET = 0x28;

static constexpr uint8_t MASTER_IDENTITY = 0x04;
static constexpr uint8_t SLAVE_IDENTITY = 0x02;

static constexpr uint8_t ICW1_8086 = 1 << 0;

static constexpr uint8_t DISABLE_MASK = 0xff;

void pic_disable()
{
    io_out8(MASTER_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);
    io_out8(SLAVE_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);

    io_out8(MASTER_DATA_SELECTOR, MASTER_OFFSET);
    io_out8(SLAVE_DATA_SELECTOR, SLAVE_OFFSET);

    io_out8(MASTER_DATA_SELECTOR, MASTER_IDENTITY);
    io_out8(SLAVE_DATA_SELECTOR, SLAVE_IDENTITY);

    io_out8(MASTER_DATA_SELECTOR, ICW1_8086);
    io_out8(SLAVE_DATA_SELECTOR, ICW1_8086);

    io_out8(MASTER_DATA_SELECTOR, DISABLE_MASK);
    io_out8(SLAVE_DATA_SELECTOR, DISABLE_MASK);

    LOG_INFO("PIC: Disabled\n");
}
