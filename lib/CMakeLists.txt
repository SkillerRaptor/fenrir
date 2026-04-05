#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/lib/bitmap.cpp)

set(HEADERS
        include/lib/bitflags.hpp
        include/lib/bitmap.hpp
        include/lib/identifier.hpp
        include/lib/math.hpp
        include/lib/queue.hpp
        include/lib/type_traits.hpp
        include/lib/types.hpp
        include/lib/vector.hpp)

add_library(lib STATIC ${SOURCES} ${HEADERS})
target_include_directories(lib PUBLIC include)
target_compile_options(lib PRIVATE ${KERNEL_BASE_OPTIONS})
target_compile_options(lib PRIVATE ${KERNEL_ARCH_OPTIONS})