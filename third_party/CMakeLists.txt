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