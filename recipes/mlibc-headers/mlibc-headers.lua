--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local binutils <const> = require("tools.binutils.binutils")
local mlibc_source <const> = require("recipes.mlibc-source.mlibc-source")
local pkgconf <const> = require("tools.pkgconf.pkgconf")
local toolchains <const> = require("recipes.toolchains.toolchains")

local MLIBC_VERSION <const> = "7.0.0"

local mlibc_headers = Package {
    name = "mlibc_headers",
    version = MLIBC_VERSION,
    revision = 1,
    source = mlibc_source,
    dependencies = {
        "base-devel",
        "meson",
        binutils,
        pkgconf,
        toolchains,
    },
    configure = [[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}" \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}" \
            -Dheaders_only=true
    ]],
    build = [[
        ninja -C ${BUILD_DIR} -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" ninja -C ${BUILD_DIR} install
    ]]
}

return mlibc_headers
