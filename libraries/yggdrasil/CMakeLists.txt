#-------------------------------------------------------------------------------------------
# Copyright (c) 2026-present, SkillerRaptor
#
# SPDX-License-Identifier: MIT
#-------------------------------------------------------------------------------------------

set(SOURCES
        src/ygg/assert.cpp
        src/ygg/bitmap.cpp
        src/ygg/format.cpp
        #src/ygg/panic.cpp
        src/ygg/string.cpp)

set(HEADERS
        include/ygg/assert.hpp
        include/ygg/atomic.hpp
        include/ygg/bitflags.hpp
        include/ygg/bitmap.hpp
        include/ygg/format.hpp
        include/ygg/hash.hpp
        include/ygg/hash_map.hpp
        include/ygg/identifier.hpp
        include/ygg/initializer_list.hpp
        include/ygg/macros.hpp
        include/ygg/math.hpp
        include/ygg/optional.hpp
        #include/ygg/panic.hpp
        include/ygg/queue.hpp
        include/ygg/result.hpp
        include/ygg/span.hpp
        include/ygg/string.hpp
        include/ygg/string_view.hpp
        include/ygg/type_traits.hpp
        include/ygg/types.hpp
        include/ygg/vector.hpp)

add_library(yggdrasil STATIC ${SOURCES} ${HEADERS})
target_include_directories(yggdrasil PUBLIC include)
target_compile_options(
        yggdrasil
        PRIVATE
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
        -fno-asynchronous-unwind-tables
        -fno-omit-frame-pointer
        -m64
        -march=x86-64
        -mabi=sysv
        -mno-80387
        -mno-mmx
        -mno-sse
        -mno-sse2)
target_link_options(
        yggdrasil
        PRIVATE
        -nostdlib
        -static
        -no-pie)