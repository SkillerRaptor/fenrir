#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/crt0.asm
        src/syscall.asm
        src/fenrir/syscall.c)

set(HEADERS
        include/stdint.h
        include/fenrir/syscall.h)

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
        -nostdlib
        -nostdinc)
target_link_options(
        libc
        PRIVATE
        -nostdlib
        -static
        -no-pie)