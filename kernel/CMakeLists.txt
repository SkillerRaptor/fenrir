#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/main.cpp
        src/acpi/acpi.cpp
        src/acpi/apic.cpp
        src/acpi/hpet.cpp
        src/arch/x86_64/cpu.cpp
        src/arch/x86_64/gdt.asm
        src/arch/x86_64/gdt.cpp
        src/arch/x86_64/idt.asm
        src/arch/x86_64/idt.cpp
        src/arch/x86_64/pic.cpp
        src/core/boot.cpp
        src/core/logger.cpp
        src/core/stacktrace.cpp
        src/drivers/serial.cpp
        src/elf/elf.cpp
        src/filesystem/ustar.cpp
        src/filesystem/vfs.cpp
        src/lib/assert.cpp
        src/lib/bitmap.cpp
        src/lib/string.cpp
        src/memory/kmalloc.cpp
        src/memory/pmm.cpp
        src/memory/vmm.cpp
        src/misc/cxxabi.cpp
        src/scheduler/reaper.cpp
        src/scheduler/scheduler.asm
        src/scheduler/scheduler.cpp
        src/scheduler/smp.cpp
        src/sync/spinlock.cpp
        src/syscalls/syscalls.asm
        src/syscalls/syscalls.cpp
        src/syscalls/syscalls/exit.cpp)

set(HEADERS
        include/acpi/acpi.hpp
        include/acpi/apic.hpp
        include/acpi/hpet.hpp
        include/arch/x86_64/cpu.hpp
        include/arch/x86_64/gdt.hpp
        include/arch/x86_64/idt.hpp
        include/arch/x86_64/io.hpp
        include/arch/x86_64/pic.hpp
        include/arch/x86_64/registers.hpp
        include/core/boot.hpp
        include/core/logger.hpp
        include/core/memory.hpp
        include/core/stacktrace.hpp
        include/drivers/serial.hpp
        include/elf/elf.hpp
        include/filesystem/ustar.hpp
        include/filesystem/vfs.hpp
        include/lib/assert.hpp
        include/lib/atomic.hpp
        include/lib/bitflags.hpp
        include/lib/bitmap.hpp
        include/lib/hash.hpp
        include/lib/hash_map.hpp
        include/lib/identifier.hpp
        include/lib/math.hpp
        include/lib/queue.hpp
        include/lib/span.hpp
        include/lib/string.hpp
        include/lib/string_view.hpp
        include/lib/type_traits.hpp
        include/lib/types.hpp
        include/lib/vector.hpp
        include/memory/kmalloc.hpp
        include/memory/mmio.hpp
        include/memory/pmm.hpp
        include/memory/vmm.hpp
        include/misc/cxxabi.hpp
        include/scheduler/process.hpp
        include/scheduler/reaper.hpp
        include/scheduler/scheduler.hpp
        include/scheduler/smp.hpp
        include/scheduler/thread.hpp
        include/scheduler/types.hpp
        include/sync/spinlock.hpp
        include/syscall/syscalls.hpp
        include/syscall/syscalls/exit.hpp)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker_scripts/x86_64.ld)

add_executable(kernel ${SOURCES} ${HEADERS})
target_include_directories(kernel PRIVATE include)
target_link_libraries(kernel PRIVATE limine flanterm nanoprintf uacpi)
set_target_properties(kernel PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

#-------------------------------------------------------------------------------------------
# Base Options
#-------------------------------------------------------------------------------------------
target_compile_options(kernel PRIVATE ${KERNEL_BASE_OPTIONS})

#-------------------------------------------------------------------------------------------
# Arch Options
#-------------------------------------------------------------------------------------------
target_compile_options(kernel PRIVATE ${KERNEL_ARCH_OPTIONS})

#-------------------------------------------------------------------------------------------
# Link Options
#-------------------------------------------------------------------------------------------
target_link_options(
        kernel
        PRIVATE
        -nostdlib
        -static
        -z max-page-size=0x1000
        --gc-sections
        -T${LINKER_SCRIPT})

#-------------------------------------------------------------------------------------------
# Iso Gen
#-------------------------------------------------------------------------------------------
set(LIMINE_DIR ${CMAKE_BINARY_DIR}/_deps/limine-src)

set(LIMINE_FILES
        ${LIMINE_DIR}/limine-bios.sys
        ${LIMINE_DIR}/limine-bios-cd.bin
        ${LIMINE_DIR}/limine-uefi-cd.bin
        ${LIMINE_DIR}/BOOTX64.EFI
        ${LIMINE_DIR}/BOOTIA32.EFI)

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/leviathan.iso

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/kernel

        COMMAND nm -Cn $<TARGET_FILE:kernel> > ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/kernel_symbols.map

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/limine.conf ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine.conf
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios.sys ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-bios.sys
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios-cd.bin ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-bios-cd.bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-uefi-cd.bin ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-uefi-cd.bin

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTX64.EFI ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT/BOOTX64.EFI
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTIA32.EFI ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT/BOOTIA32.EFI

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/applications
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hello_world> ${CMAKE_CURRENT_BINARY_DIR}/applications/

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/services
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:window_service> ${CMAKE_CURRENT_BINARY_DIR}/services/

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/modules
        COMMAND tar --sort=name --owner=0 --group=0 --numeric-owner --format=ustar -cf ${CMAKE_CURRENT_BINARY_DIR}/iso_root/modules/initramfs.tar applications/ services/

        COMMAND xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label ${CMAKE_CURRENT_BINARY_DIR}/iso_root -o ${CMAKE_BINARY_DIR}/leviathan.iso

        DEPENDS kernel hello_world ${CMAKE_SOURCE_DIR}/limine.conf ${LINKER_SCRIPT} ${LIMINE_FILES}

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
        iso
        ALL
        DEPENDS ${CMAKE_BINARY_DIR}/leviathan.iso kernel hello_world)

#-------------------------------------------------------------------------------------------
# Run
#-------------------------------------------------------------------------------------------
add_custom_target(run
        COMMAND qemu-system-x86_64
        -M q35,smm=off
        -boot d
        -cdrom leviathan.iso
        --no-reboot
        --no-shutdown
        -d int
        -m 256M
        -serial stdio
        -smp 4
        DEPENDS iso
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})