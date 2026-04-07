#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------
# CMake Info
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)
project(Leviathan LANGUAGES ASM_NASM C CXX)

#-------------------------------------------------------------------------------------------
# Standard Settings
#-------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------------------
# Build Options
#-------------------------------------------------------------------------------------------
set(KERNEL_BASE_OPTIONS
        -O0
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
        -fno-exceptions
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables)

set(KERNEL_ARCH_OPTIONS
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
# Third Party
#-------------------------------------------------------------------------------------------
add_subdirectory(third_party)

#-------------------------------------------------------------------------------------------
# Userland
#-------------------------------------------------------------------------------------------
add_subdirectory(userland)

#-------------------------------------------------------------------------------------------
# Kernel
#-------------------------------------------------------------------------------------------
add_subdirectory(kernel)