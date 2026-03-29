/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/pic.hpp"

#include "kernel/arch/x86_64/io.hpp"
#include "kernel/core/logger.hpp"

namespace kernel::pic {

static constexpr u16 s_master_command_selector = 0x20;
static constexpr u16 s_master_data_selector = 0x21;

static constexpr u16 s_slave_command_selector = 0xa0;
static constexpr u16 s_slave_data_selector = 0xa1;

static constexpr u8 s_icw1_init = 1 << 4;
static constexpr u8 s_icw1_icw4 = 1 << 0;

static constexpr u8 s_master_offset = 0x20;
static constexpr u8 s_slave_offset = 0x28;

static constexpr u8 s_master_identity = 0x04;
static constexpr u8 s_slave_identity = 0x02;

static constexpr u8 s_icw1_8086 = 1 << 0;

static constexpr u8 s_mask = 0xff;

void disable()
{
    logger::debug("PIC: Disabling...\n");

    io::out8(s_master_command_selector, s_icw1_init | s_icw1_icw4);
    io::out8(s_slave_command_selector, s_icw1_init | s_icw1_icw4);
    io::wait();

    io::out8(s_master_data_selector, s_master_offset);
    io::out8(s_slave_data_selector, s_slave_offset);
    io::wait();

    io::out8(s_master_data_selector, s_master_identity);
    io::out8(s_slave_data_selector, s_slave_identity);
    io::wait();

    io::out8(s_master_data_selector, s_icw1_8086);
    io::out8(s_slave_data_selector, s_icw1_8086);
    io::wait();

    io::out8(s_master_data_selector, s_mask);
    io::out8(s_slave_data_selector, s_mask);
    io::wait();

    logger::info("PIC: Disabled\n");
}

} // namespace kernel::pic
