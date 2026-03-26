#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
    src/main.cpp
    src/misc/cxxabi.cpp)

set(HEADERS
    include/misc/cxxabi.hpp)

add_executable(kernel ${SOURCES} ${HEADERS})
target_include_directories(kernel PRIVATE include)
target_link_libraries(kernel PRIVATE limine)

#-------------------------------------------------------------------------------------------
# Base Options
#-------------------------------------------------------------------------------------------
target_compile_options(
        kernel
        PRIVATE
        -g
        -Wall
        -Wextra
        -ffreestanding
        -fno-stack-protector
        -fno-stack-check
        -fno-lto
        -fno-PIC
        -ffunction-sections
        -fdata-sections
        -fno-rtti
        -fno-exceptions)

#-------------------------------------------------------------------------------------------
# Arch Options
#-------------------------------------------------------------------------------------------
target_compile_options(
        kernel
        PRIVATE
        -m64
        -march=x86-64
        -mabi=sysv
        -mno-80387
        -mno-mmx
        -mno-sse
        -mno-sse2
        -mno-red-zone
        -mcmodel=kernel)

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
        -T${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/linker.ld)

#-------------------------------------------------------------------------------------------
# Iso Gen
#-------------------------------------------------------------------------------------------
set(LIMINE_DIR ${CMAKE_BINARY_DIR}/_deps/limine-src)

add_custom_command(
        OUTPUT leviathan.iso

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/kernel

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/limine.conf ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine.conf
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios.sys ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-bios.sys
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-bios-cd.bin ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-bios-cd.bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/limine-uefi-cd.bin ${CMAKE_CURRENT_BINARY_DIR}/iso_root/boot/limine/limine-uefi-cd.bin

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTX64.EFI ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT/BOOTX64.EFI
        COMMAND ${CMAKE_COMMAND} -E copy ${LIMINE_DIR}/BOOTIA32.EFI ${CMAKE_CURRENT_BINARY_DIR}/iso_root/EFI/BOOT/BOOTIA32.EFI

        COMMAND xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label ${CMAKE_CURRENT_BINARY_DIR}/iso_root -o ${CMAKE_BINARY_DIR}/leviathan.iso
        DEPENDS kernel ${CMAKE_SOURCE_DIR}/limine.conf
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
        iso
        ALL
        DEPENDS leviathan.iso)

#-------------------------------------------------------------------------------------------
# Run
#-------------------------------------------------------------------------------------------
add_custom_target(run
        COMMAND qemu-system-x86_64
        -M q35
        -boot d
        -cdrom leviathan.iso
        --no-reboot
        --no-shutdown
        -d int
        -m 256M
        -monitor stdio
        DEPENDS iso
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})