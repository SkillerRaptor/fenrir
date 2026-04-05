#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/kernel/main.cpp
        src/kernel/acpi/acpi.cpp
        src/kernel/acpi/apic.cpp
        src/kernel/acpi/hpet.cpp
        src/kernel/arch/x86_64/gdt.asm
        src/kernel/arch/x86_64/gdt.cpp
        src/kernel/arch/x86_64/idt.asm
        src/kernel/arch/x86_64/idt.cpp
        src/kernel/arch/x86_64/pic.cpp
        src/kernel/core/boot.cpp
        src/kernel/core/logger.cpp
        src/kernel/core/stacktrace.cpp
        src/kernel/drivers/serial.cpp
        src/kernel/memory/kmalloc.cpp
        src/kernel/memory/pmm.cpp
        src/kernel/memory/vmm.cpp
        src/kernel/misc/cxxabi.cpp
        src/kernel/scheduler/scheduler.asm
        src/kernel/scheduler/scheduler.cpp
        src/kernel/scheduler/smp.cpp
        src/kernel/sync/spinlock.cpp)

set(HEADERS
        include/kernel/acpi/acpi.hpp
        include/kernel/acpi/apic.hpp
        include/kernel/acpi/hpet.hpp
        include/kernel/arch/x86_64/cpu.hpp
        include/kernel/arch/x86_64/gdt.hpp
        include/kernel/arch/x86_64/idt.hpp
        include/kernel/arch/x86_64/io.hpp
        include/kernel/arch/x86_64/pic.hpp
        include/kernel/arch/x86_64/registers.hpp
        include/kernel/core/boot.hpp
        include/kernel/core/logger.hpp
        include/kernel/core/memory.hpp
        include/kernel/core/stacktrace.hpp
        include/kernel/drivers/serial.hpp
        include/kernel/memory/kmalloc.hpp
        include/kernel/memory/mmio.hpp
        include/kernel/memory/pmm.hpp
        include/kernel/memory/vmm.hpp
        include/kernel/misc/cxxabi.hpp
        include/kernel/scheduler/process.hpp
        include/kernel/scheduler/scheduler.hpp
        include/kernel/scheduler/smp.hpp
        include/kernel/scheduler/thread.hpp
        include/kernel/scheduler/types.hpp
        include/kernel/sync/spinlock.hpp)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker_scripts/x86_64.ld)

add_executable(kernel ${SOURCES} ${HEADERS})
target_include_directories(kernel PRIVATE include)
target_link_libraries(kernel PRIVATE lib klibc limine flanterm nanoprintf uacpi)
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

        COMMAND xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label ${CMAKE_CURRENT_BINARY_DIR}/iso_root -o ${CMAKE_BINARY_DIR}/leviathan.iso

        DEPENDS kernel ${CMAKE_SOURCE_DIR}/limine.conf ${LINKER_SCRIPT} ${LIMINE_FILES}

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
        iso
        ALL
        DEPENDS ${CMAKE_BINARY_DIR}/leviathan.iso kernel)

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