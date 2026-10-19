--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local BINUTILS_VERSION <const> = "2.46.0"

local binutils = Tool {
    name = "binutils",
    version = BINUTILS_VERSION,
    revision = 1,
    source = Source {
        name = "binutils",
        Archive (
            string.gsub("https://mirrors.ocf.berkeley.edu/gnu/binutils/binutils-${version}.tar.gz", "${version}", BINUTILS_VERSION),
            "8608fe44ab7de645f6ad0a898313b75338842490d609adb85c9fb2827c376af2"
        ),
        patches = {
            "tools/binutils/patches/0001.patch"
        }
    },
    dependencies = {
        "base-devel"
    },
    configure = [[
        "${SOURCE_DIR}"/configure \
            --target=${ARCH}-fenrir \
            --prefix="${PREFIX}" \
            --with-sysroot="${SYSROOT_DIR}" \
            --disable-werror \
            --enable-default-execstack=no
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install
    ]]
}

return binutils
