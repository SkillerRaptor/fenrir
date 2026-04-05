/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/acpi/acpi.hpp"
#include "kernel/acpi/apic.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/gdt.hpp"
#include "kernel/arch/x86_64/idt.hpp"
#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/stacktrace.hpp"
#include "kernel/drivers/serial.hpp"
#include "kernel/memory/pmm.hpp"
#include "kernel/memory/vmm.hpp"
#include "kernel/misc/cxxabi.hpp"
#include "kernel/scheduler/scheduler.hpp"
#include "kernel/scheduler/smp.hpp"

namespace kernel {

static void kmain_thread(void *user_argument);

extern "C" void kmain()
{
    cpu::disable_interrupts();

    if (!boot::is_base_revision_supported()) {
        cpu::halt();
    }

    serial::initialize();
    logger::initialize();

    logger::log("\n");
    logger::log("          _______ _    _ _____ _______ _______ _     _ _______ __   _\n");
    logger::log("   |      |______  \\  /    |   |_____|    |    |_____| |_____| | \\  |\n");
    logger::log("   |_____ |______   \\/   __|__ |     |    |    |     | |     | |  \\_|\n");
    logger::log("\n");
    logger::log("   Bootloader: %s %s\n", boot::get_bootloader_name(), boot::get_bootloader_version());
    logger::log("   Firmware: %s\n", boot::get_firmware_type());
    logger::log("\n");

    idt::initialize();

    pmm::initialize();
    vmm::initialize();

    cxxabi::construct();

    stacktrace::initialize();

    acpi::initialize();
    hpet::initialize();
    apic::initialize();

    scheduler::initialize();
    smp::initialize();

    scheduler::create_thread(scheduler::get_kernel_process(), 0x28, kmain_thread, nullptr);

    scheduler::yield();
}

void kmain_thread(void *)
{
    logger::info("Leviathan successfully booted!\n");

    scheduler::yield();
}

} // namespace kernel
