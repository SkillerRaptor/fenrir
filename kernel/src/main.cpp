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
#include "filesystem/vfs.hpp"
#include "lib/math.hpp"
#include "lib/string.hpp"
#include "lib/string_view.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "misc/cxxabi.hpp"
#include "scheduler/reaper.hpp"
#include "scheduler/scheduler.hpp"
#include "scheduler/smp.hpp"
#include "syscall/syscalls.hpp"

[[noreturn]] static void kmain_thread();

extern "C" void kmain()
{
    cpu::disable_interrupts();

    if (!boot::is_base_revision_supported()) {
        cpu::halt();
    }

    serial::initialize();
    logger::initialize();

    // FIXME: This is a hack
    cpu::initialize(0);

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

    syscalls::initialize();

    scheduler::initialize();
    smp::initialize();

    scheduler::create_thread(scheduler::get_kernel_process(), 0x28, reaper::run);
    scheduler::create_thread(scheduler::get_kernel_process(), 0x28, kmain_thread);

    scheduler::yield();
}

[[noreturn]] void kmain_thread()
{
    logger::info("Leviathan successfully booted!\n");

    limine_file *module = boot::get_modules()[1]; // initramfs.tar

    logger::info("File: %s\n", module->path);

    vfs::mount(module, "/", "USTAR");

    vfs::File *hello_world_file = vfs::open("/applications/hello_world");

    vfs::seek(hello_world_file, 0, vfs::SeekOrigin::End);
    const usize size = vfs::tell(hello_world_file);
    vfs::seek(hello_world_file, 0, vfs::SeekOrigin::Set);

    u8 *bytes = new u8[size];
    vfs::read(hello_world_file, bytes, size);

    vfs::close(hello_world_file);

    const elf::Elf elf(bytes);

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

        for (usize j = 0; j < page_count; ++j) {
            const u64 physical_address = reinterpret_cast<u64>(pmm::allocate(1, true));
            const u64 virtual_address = virtual_start + j * memory::s_page_size;

            vmm::Attribute attr = vmm::Attribute::User;
            if ((program_header.flags & elf::ProgramHeader::Flags::Writeable) == elf::ProgramHeader::Flags::Writeable) {
                attr = attr | vmm::Attribute::Write;
            }

            vmm::map(user_page_map, physical_address, virtual_address, attr);
        }

        const u8 *src = bytes + program_header.offset;
        u8 *dst = reinterpret_cast<u8 *>(
            vmm::virtual_to_physical(user_page_map, program_header.virtual_address) + boot::get_hhdm_offset());

        memcpy(dst, src, program_header.file_size);
    }

    logger::info("Creating user process...\n");

    Process *user_process = scheduler::create_process(user_page_map);
    scheduler::create_thread(user_process, 0x40 | 3, reinterpret_cast<void (*)()>(elf.header().entry));

    scheduler::yield();
}
