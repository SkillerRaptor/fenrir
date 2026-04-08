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
#include "core/memory.hpp"
#include "core/stacktrace.hpp"
#include "drivers/serial.hpp"
#include "elf/elf.hpp"
#include "lib/math.hpp"
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

    const limine_module_response *response = boot::get_module_response();
    const limine_file *hello_world = response->modules[1];

    const u8 *data = static_cast<const u8 *>(hello_world->address);
    const elf::Elf elf(data);

    vmm::PageMap *user_page_map = vmm::create_page_map();

    for (const elf::ProgramHeader &program_header : elf.program_headers()) {
        if (program_header.memory_size == 0) {
            continue;
        }

        if (program_header.type != elf::ProgramHeader::Type::Load) {
            continue;
        }

        const u64 virtual_start = math::align_down(program_header.virtual_address, memory::s_page_size);
        const u64 virtual_end
            = math::align_up(program_header.virtual_address + program_header.memory_size, memory::s_page_size);
        const usize page_count = (virtual_end - virtual_start) / memory::s_page_size;

        for (usize j { 0 }; j < page_count; ++j) {
            const u64 physical_address = reinterpret_cast<u64>(pmm::allocate(1, true));
            const u64 virtual_address = virtual_start + j * memory::s_page_size;

            vmm::Attribute attr = vmm::Attribute::User;
            if ((program_header.flags & elf::ProgramHeader::Flags::Writeable) == elf::ProgramHeader::Flags::Writeable) {
                attr = attr | vmm::Attribute::Write;
            }

            vmm::map(user_page_map, physical_address, virtual_address, attr);
        }

        const u8 *src = data + program_header.offset;
        u8 *dst = reinterpret_cast<u8 *>(
            vmm::virtual_to_physical(user_page_map, program_header.virtual_address) + boot::get_hhdm_offset());

        memcpy(dst, src, program_header.file_size);
    }

    const ProcessId user_process = scheduler::create_process(user_page_map);
    scheduler::create_thread(user_process, 0x40 | 3, reinterpret_cast<void (*)()>(elf.header().entry));

    scheduler::yield();
}
