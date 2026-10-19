--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local binutils <const> = require("chariot.tools.binutils.binutils")
local mlibc_headers <const> = require("chariot.recipes.mlibc-headers.mlibc-headers")

local GCC_VERSION <const> = "14.4.0"

local gcc = Tool {
    name = "gcc",
    version = GCC_VERSION,
    revision = 1,
    source = Source {
        name = "gcc",
        Archive (
            string.gsub("https://mirrors.ocf.berkeley.edu/gnu/gcc/gcc-${version}/gcc-${version}.tar.gz", "${version}", GCC_VERSION),
            "f35aa1c7f14e6bbe5a7fab83abcb7813449aa1dc6d38bbf27b74614dee17db5e"
        ),
        patches = {
            "chariot/tools/gcc/patches/0001.patch"
        }
    },
    dependencies = {
        "base-devel",
        binutils,
        mlibc_headers,
    },
    configure = [[
        CFLAGS_FOR_TARGET="-march=x86-64 -mabi=sysv" \
        CXXFLAGS_FOR_TARGET="-march=x86-64 -mabi=sysv" \
        "${SOURCE_DIR}"/configure \
            --target=${ARCH}-fenrir \
            --prefix="${PREFIX}" \
            --with-sysroot="${SYSROOT_DIR}" \
            --enable-languages=c,c++ \
            --enable-threads=posix \
            --disable-multilib \
            --enable-shared \
            --enable-host-shared
    ]],
    build = [[
        make -j"${PARALLELISM}" all-gcc all-target-libgcc
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install-gcc install-target-libgcc
    ]]
}

return gcc
