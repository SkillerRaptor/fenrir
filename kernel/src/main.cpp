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
#include "lib/string.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "misc/cxxabi.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/smp.hpp"
#include "syscall/syscalls.hpp"

static void kmain_thread();

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

    scheduler::create_thread(scheduler::get_kernel_process(), 0x28, kmain_thread);

    scheduler::yield();
}

void kmain_thread()
{
    logger::info("Leviathan successfully booted!\n");

    vmm::PageMap *user_page_map = vmm::create_page_map();
    void *code_page_phys = pmm::allocate(1, true);
    const u64 code_phys = reinterpret_cast<u64>(code_page_phys);

    static constexpr u8 s_code[] = {
        // mov eax, 0x646e61
        0xb8,
        0x61,
        0x6e,
        0x64,
        0x00,
        // push rax
        0x50,
        // movabs, rax, 0x6c72657355206d6f
        0x48,
        0xb8,
        0x6f,
        0x6d,
        0x20,
        0x55,
        0x73,
        0x65,
        0x72,
        0x6c,
        // push rax
        0x50,
        // movabs rax, 0x7266206f6c6c6548
        0x48,
        0xb8,
        0x48,
        0x65,
        0x6c,
        0x6c,
        0x6f,
        0x20,
        0x66,
        0x72,
        // push rax
        0x50,
        // mov eax, 0x1
        0xb8,
        0x01,
        0x00,
        0x00,
        0x00,
        // mov edi, 0x1
        0xbf,
        0x01,
        0x00,
        0x00,
        0x00,
        // mov rsi, rsp
        0x48,
        0x89,
        0xe6,
        // mov edx, 0x13
        0xba,
        0x13,
        0x00,
        0x00,
        0x00,
        // syscall
        0x0f,
        0x05,
        // jmp 0x30
        0xeb,
        0xfe,
    };
    u8 *code = reinterpret_cast<u8 *>(code_phys + boot::get_hhdm_offset());
    memcpy(code, s_code, sizeof(s_code));

    vmm::map(user_page_map, code_phys, 0x1000, vmm::Attribute::User | vmm::Attribute::Write);

    const ProcessId user_process = scheduler::create_process(user_page_map);
    scheduler::create_thread(user_process, 0x40 | 3, reinterpret_cast<void (*)()>(0x1000));

    scheduler::yield();
}
