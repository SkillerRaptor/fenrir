--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local binutils <const> = require("chariot.tools.binutils.binutils")
local gcc <const> = require("chariot.tools.gcc.gcc")
local mlibc_headers <const> = require("chariot.recipes.mlibc-headers.mlibc-headers")
local mlibc_source <const> = require("chariot.recipes.mlibc-source.mlibc-source")
local pkgconf <const> = require("chariot.tools.pkgconf.pkgconf")
local toolchains <const> = require("chariot.recipes.toolchains.toolchains")

local MLIBC_VERSION <const> = "7.0.0"

local frigg = Package {
    name = "frigg",
    version = "0.0",
    revision = 1,
    source = Source {
        name = "frigg",
        Git("https://github.com/managarm/frigg.git", "b9ce1199489f25add06d66c5cc355e2c17dbe027")
    },
    dependencies = {
        "base-devel",
        "meson",
        binutils,
        gcc,
        pkgconf,
        toolchains,
    },
    configure = [[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}" \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}" \
            -Dbuild_tests=disabled
    ]],
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" meson install -C ${BUILD_DIR} --strip
    ]]
}

local libsmarter = Package {
    name = "libsmarter",
    version = "0.0",
    revision = 1,
    source = Source {
        name = "libsmarter",
        Git("https://github.com/managarm/libsmarter.git", "f7d061bc37d485418344452c7ceb28d5df3ba85d")
    },
    dependencies = {
        "base-devel",
        "meson",
        binutils,
        gcc,
        pkgconf,
        toolchains,
    },
    configure = [[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}" \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}"
    ]],
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" meson install -C ${BUILD_DIR}
    ]]
}

local c_hdrs = Package {
    name = "freestnd-c-hdrs",
    version = "0.0",
    revision = 1,
    source = Source {
        name = "freestnd-c-hdrs",
        Git("https://github.com/osdev0/freestnd-c-hdrs.git", "d33711241b46ecb8f2ad33927fcefdcb3ac0162e")
    },
    dependencies = {
        "base-devel",
        binutils,
        gcc,
        pkgconf,
    },
    install = [[
        install -d "${INSTALL_DIR}${PREFIX}/include/freestnd-c-hdrs" "${INSTALL_DIR}${PREFIX}/lib/pkgconfig"
        cp -r "${SOURCE_DIR}/${ARCH}/include/." "${INSTALL_DIR}${PREFIX}/include/freestnd-c-hdrs/"

        pc="${INSTALL_DIR}${PREFIX}/lib/pkgconfig/freestnd-c-hdrs-${ARCH}.pc"
        echo "prefix=${PREFIX}"                                    >> "${pc}"
        echo 'includedir=${prefix}/include'                        >> "${pc}"
        echo "Name: freestnd-c-hdrs-${ARCH}"                       >> "${pc}"
        echo "Description: freestanding headers for mlibc"         >> "${pc}"
        echo "Version: 0"                                          >> "${pc}"
        echo 'Cflags: -idirafter${includedir}/freestnd-c-hdrs'     >> "${pc}"
    ]],
}

local cxx_hdrs = Package {
    name = "freestnd-cxx-hdrs",
    version = "0.0",
    revision = 1,
    source = Source {
        name = "freestnd-cxx-hdrs",
        Git("https://github.com/osdev0/freestnd-cxx-hdrs.git", "a6b351e0ab3e74e5789b01fa1447e4cd62373da7")
    },
    dependencies = {
        "base-devel",
        binutils,
        gcc,
        pkgconf,
    },
    install = [[
        install -d "${INSTALL_DIR}${PREFIX}/include/freestnd-cxx-hdrs" "${INSTALL_DIR}${PREFIX}/lib/pkgconfig"
        cp -r "${SOURCE_DIR}/${ARCH}/include/." "${INSTALL_DIR}${PREFIX}/include/freestnd-cxx-hdrs/"

        pc="${INSTALL_DIR}${PREFIX}/lib/pkgconfig/freestnd-cxx-hdrs-${ARCH}.pc"
        echo "prefix=${PREFIX}"                                      >> "${pc}"
        echo 'includedir=${prefix}/include'                          >> "${pc}"
        echo "Name: freestnd-cxx-hdrs-${ARCH}"                       >> "${pc}"
        echo "Description: freestanding headers for mlibc"           >> "${pc}"
        echo "Version: 0"                                            >> "${pc}"
        echo 'Cflags: -isystem${includedir}/freestnd-cxx-hdrs'       >> "${pc}"
    ]],
}

local mlibc = Package {
    name = "mlibc",
    version = MLIBC_VERSION,
    revision = 1,
    source = mlibc_source,
    dependencies = {
        "base-devel",
        "git",
        "meson",

        frigg,
        libsmarter,
        c_hdrs,
        cxx_hdrs,
        binutils,
        gcc,
        mlibc_headers,
        pkgconf,
        toolchains,
    },
    configure = [[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}" \
            --cross-file="${SOURCES_DIR}/toolchains/${ARCH}-fenrir.ini" \
            --prefix="${PREFIX}" \
            -Ddefault_library=static \
            -Ddebug=false \
            -Dno_headers=true
    ]],
    build = [[
        ninja -C ${BUILD_DIR} -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" ninja -C ${BUILD_DIR} install
    ]]
}

return mlibc
