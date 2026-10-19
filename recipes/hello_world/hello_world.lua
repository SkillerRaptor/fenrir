--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local binutils <const> = require("tools.binutils.binutils")
local gcc <const> = require("tools.gcc.gcc")
local mlibc <const> = require("recipes.mlibc.mlibc")
local mlibc_headers <const> = require("recipes.mlibc-headers.mlibc-headers")
local toolchains <const> = require("recipes.toolchains.toolchains")

local hello_world = Package {
    name = "hello_world",
    version = "0",
    revision = 1,
    source = Source {
        name = "hello_world",
        Local("userland/applications/hello_world"),
    },
    dependencies = {
        "base-devel",
        "meson",
        "ninja",
        binutils,
        gcc,
        mlibc,
        mlibc_headers,
        toolchains,
    },
    configure = [[
        meson setup ${BUILD_DIR} ${SOURCE_DIR} \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}"
    ]],
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        cp "${BUILD_DIR}/hello_world" "${INSTALL_DIR}"
    ]]
}

return hello_world
