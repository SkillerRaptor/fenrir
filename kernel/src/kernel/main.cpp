/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/acpi/acpi.hpp"
#include "kernel/acpi/apic.hpp"
#include "kernel/acpi/hpet.hpp"
#include "kernel/arch/x86_64/cpu.hpp"
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
#include "kernel/syscall/syscalls.hpp"

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

} // namespace kernel
