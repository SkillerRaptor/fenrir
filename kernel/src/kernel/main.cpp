/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/gdt.hpp"
#include "kernel/arch/x86_64/idt.hpp"
#include "kernel/arch/x86_64/pic.hpp"
#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/memory/pmm.hpp"
#include "kernel/misc/cxxabi.hpp"

namespace kernel {

__attribute__((noreturn)) extern "C" void kmain()
{
    cpu::disable_interrupts();

    if (!boot::is_base_revision_supported()) {
        cpu::halt();
    }

    logger::initialize();

    logger::log("\n");
    logger::log("          _______ _    _ _____ _______ _______ _     _ _______ __   _\n");
    logger::log("   |      |______  \\  /    |   |_____|    |    |_____| |_____| | \\  |\n");
    logger::log("   |_____ |______   \\/   __|__ |     |    |    |     | |     | |  \\_|\n");
    logger::log("\n");
    logger::log("   Bootloader: %s %s\n", boot::get_bootloader_name(), boot::get_bootloader_version());
    logger::log("   Firmware: %s\n", boot::get_firmware_type());
    logger::log("\n");

    gdt::initialize();
    pic::remap();
    idt::initialize();

    pmm::initialize();

    cpu::enable_interrupts();

    cxxabi::construct();

    while (true) {
        cpu::disable_interrupts();
        cpu::halt();
    }
}

} // namespace kernel
