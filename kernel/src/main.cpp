/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "acpi/acpi.hpp"
#include "acpi/apic.hpp"
#include "acpi/hpet.hpp"
#include "arch/x86_64/cpu.hpp"
#include "arch/x86_64/idt.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "drivers/serial.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "misc/cxxabi.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/smp.hpp"
#include "syscall/syscalls.hpp"

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

    cxxabi::construct();

    idt::initialize();

    pmm::initialize();
    vmm::initialize();

    stacktrace::initialize();

    acpi::initialize();
    hpet::initialize();
    apic::initialize();

    scheduler::initialize();
    smp::initialize();

    syscalls::initialize();

    vmm::PageMap *user_page_map = vmm::create_page_map();
    void *code_page_phys = pmm::allocate(1, true);
    const u64 code_phys = reinterpret_cast<u64>(code_page_phys);

    u8 *code_virt = reinterpret_cast<u8 *>(code_phys + boot::get_hhdm_offset());
    code_virt[0] = 0xb8; // mov eax, 0x45
    code_virt[1] = 0x45;
    code_virt[2] = 0x00;
    code_virt[3] = 0x00;
    code_virt[4] = 0x00;

    code_virt[5] = 0x0f; // syscall
    code_virt[6] = 0x05;

    code_virt[7] = 0xeb; // jmp 7
    code_virt[8] = 0xfe;

    vmm::map(user_page_map, code_phys, 0x1000, vmm::Attribute::User | vmm::Attribute::Write);

    ProcessId user_process = scheduler::create_process(user_page_map);
    ThreadId user_thread
        = scheduler::create_thread(user_process, 0x40 | 3, reinterpret_cast<void (*)(void *)>(0x1000), nullptr);

    scheduler::create_thread(scheduler::get_kernel_process(), 0x28, kmain_thread, nullptr);

    scheduler::yield();
}

void kmain_thread(void *)
{
    logger::info("Leviathan successfully booted!\n");

    scheduler::yield();
}
