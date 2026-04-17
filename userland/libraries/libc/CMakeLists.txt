#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/crt0.asm
        src/syscall.asm
        src/syscalls.c)

set(HEADERS
        include/syscalls.h)

add_library(libc STATIC ${SOURCES} ${HEADERS})
target_include_directories(libc PUBLIC include)
target_compile_options(
        libc
        PRIVATE
        -ffreestanding
        -fno-stack-protector
        -fno-pie
        -fno-pic
        -m64
        -nostdlib)
target_link_options(
        libc
        PRIVATE
        -nostdlib
        -static
        -no-pie)