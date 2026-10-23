--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local limine_headers <const> = require("recipes.limine-headers.limine-headers")

local kernel = Package {
    name = "kernel",
    version = "0",
    revision = 1,
    source = Source {
        name = "kernel",
        Local("kernel/"),
    },
    dependencies = {
        "base-devel",
        "meson",
        "ninja",
        "nasm",
        limine_headers,
    },
    configure = [[
        meson setup ${BUILD_DIR} ${SOURCE_DIR} \
            --prefix="${PREFIX}" \
            -Dsources_dir="${SOURCES_DIR}"
    ]],
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        mkdir -p "${INSTALL_DIR}/boot"
        cp "${BUILD_DIR}/fenrir" "${INSTALL_DIR}/boot/fenrir"
    ]]
}

return kernel
