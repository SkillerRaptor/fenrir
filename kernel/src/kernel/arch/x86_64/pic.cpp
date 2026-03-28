/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/pic.hpp"

#include "kernel/arch/x86_64/io.hpp"
#include "kernel/core/logger.hpp"

namespace kernel::pic {

#define MASTER_COMMAND_SELECTOR 0x20
#define MASTER_DATA_SELECTOR 0x21
#define MASTER_OFFSET 0x20
#define MASTER_IDENTITY 0x04

#define SLAVE_COMMAND_SELECTOR 0xa0
#define SLAVE_DATA_SELECTOR 0xa1
#define SLAVE_OFFSET 0x28
#define SLAVE_IDENTITY 0x02

#define ICW1_ICW4 (1 << 0)
#define ICW1_INIT (1 << 4)
#define ICW1_8086 (1 << 0)

void remap()
{
    logger::info("PIC: Remapping...\n");

    io::out8(MASTER_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);
    io::wait();
    io::out8(SLAVE_COMMAND_SELECTOR, ICW1_INIT | ICW1_ICW4);
    io::wait();

    io::out8(MASTER_DATA_SELECTOR, MASTER_OFFSET);
    io::wait();
    io::out8(SLAVE_DATA_SELECTOR, SLAVE_OFFSET);
    io::wait();

    io::out8(MASTER_DATA_SELECTOR, MASTER_IDENTITY);
    io::wait();
    io::out8(SLAVE_DATA_SELECTOR, SLAVE_IDENTITY);
    io::wait();

    io::out8(MASTER_DATA_SELECTOR, ICW1_8086);
    io::wait();
    io::out8(SLAVE_DATA_SELECTOR, ICW1_8086);
    io::wait();

    logger::ok("PIC: Remapped\n");
}

} // namespace kernel::pic
