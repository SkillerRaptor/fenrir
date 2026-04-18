#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/main.c)

set(HEADERS
)

add_executable(hello_world ${SOURCES} ${HEADERS})
target_link_libraries(hello_world PRIVATE libc)
target_compile_options(
        hello_world
        PRIVATE
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