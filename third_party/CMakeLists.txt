#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

#-------------------------------------------------------------------------------------------
# Limine
#-------------------------------------------------------------------------------------------
CPMAddPackage(
        NAME Limine
        GIT_REPOSITORY https://codeberg.org/Limine/Limine.git
        GIT_TAG v11.1.0-binary
        DOWNLOAD_ONLY YES)

#-------------------------------------------------------------------------------------------
# Limine Protocol
#-------------------------------------------------------------------------------------------
CPMAddPackage(
        NAME Limine-Protocol
        GIT_REPOSITORY https://codeberg.org/Limine/limine-protocol.git
        GIT_TAG trunk
        DOWNLOAD_ONLY YES)

if (Limine-Protocol_ADDED)
    add_library(limine INTERFACE)
    target_include_directories(limine INTERFACE ${Limine-Protocol_SOURCE_DIR}/include)
endif ()

#-------------------------------------------------------------------------------------------
# Flanterm
#-------------------------------------------------------------------------------------------
CPMAddPackage(
        NAME Flanterm
        GIT_REPOSITORY https://codeberg.org/Mintsuki/Flanterm.git
        GIT_TAG v3.0.2
        DOWNLOAD_ONLY YES)

if (Flanterm_ADDED)
    add_library(flanterm STATIC ${Flanterm_SOURCE_DIR}/src/flanterm.c ${Flanterm_SOURCE_DIR}/src/flanterm_backends/fb.c)
    target_include_directories(flanterm PUBLIC ${Flanterm_SOURCE_DIR}/src)
    target_compile_options(flanterm PRIVATE ${KERNEL_BASE_OPTIONS} ${KERNEL_ARCH_OPTIONS})
endif ()

#-------------------------------------------------------------------------------------------
# nanoprintf
#-------------------------------------------------------------------------------------------
CPMAddPackage(
        NAME nanoprintf
        GIT_REPOSITORY https://github.com/charlesnicholson/nanoprintf.git
        GIT_TAG v0.5.6
        DOWNLOAD_ONLY YES)

if (nanoprintf_ADDED)
    add_library(nanoprintf INTERFACE)
    target_include_directories(nanoprintf INTERFACE ${nanoprintf_SOURCE_DIR})
endif ()