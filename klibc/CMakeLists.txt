#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/string.cpp)

set(HEADERS
        include/string.h)

add_library(klibc STATIC ${SOURCES} ${HEADERS})
target_include_directories(klibc PUBLIC include)
target_compile_options(klibc PRIVATE ${KERNEL_BASE_OPTIONS})
target_compile_options(klibc PRIVATE ${KERNEL_ARCH_OPTIONS})