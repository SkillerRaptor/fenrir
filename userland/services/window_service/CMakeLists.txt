#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/main.cpp)

set(HEADERS
)

add_executable(window_service ${SOURCES} ${HEADERS})
target_link_libraries(window_service PRIVATE libc)
target_compile_options(
        window_service
        PRIVATE
        -fno-stack-protector
        -fno-pie
        -fno-pic
        -m64
        -nostdlib
        -fno-rtti
        -fno-exceptions)
target_link_options(
        window_service
        PRIVATE
        -nostdlib
        -static
        -no-pie)