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

local doomgeneric = Package {
    name = "doomgeneric",
    version = "0",
    revision = 1,
    source = Source {
        name = "doomgeneric",
        Git("https://github.com/ozkl/doomgeneric.git", "dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284"),
        patches = {
            "recipes/doomgeneric/patches/0001.patch"
        }
    },
    dependencies = {
        "base-devel",
        "meson",
        "ninja",
        "wget",
        binutils,
        gcc,
        mlibc,
        mlibc_headers,
        toolchains,
    },
    configure = [[
        wget -O ./DOOM1.WAD.gz "https://archive.org/download/wadarchive/DATA/5b.zip/5b%2F2e249b9c5133ec987b3ea77596381dc0d6bc1d%2F5b2e249b9c5133ec987b3ea77596381dc0d6bc1d.wad.gz"
        gunzip -f ./DOOM1.WAD.gz

        meson setup ${BUILD_DIR} ${SOURCE_DIR} \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}"
    ]],
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        cp "${BUILD_DIR}/doomgeneric" "${INSTALL_DIR}"
        cp ./DOOM1.WAD "${INSTALL_DIR}"
    ]]
}

return doomgeneric
