#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/crt0.asm
        src/main.c
        src/syscall.asm)

set(HEADERS
)

add_executable(hello_world ${SOURCES} ${HEADERS})
target_compile_options(
        hello_world
        PRIVATE
        -ffreestanding
        -fno-stack-protector
        -fno-pie
        -fno-pic
        -m64
        -nostdlib)
target_link_options(
        hello_world
        PRIVATE
        -nostdlib
        -static
        -no-pie)