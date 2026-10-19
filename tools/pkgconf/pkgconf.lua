--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local PKGCONF_VERSION <const> = "3.0.10"

local pkgconf = Tool {
    name = "pkgconf",
    version = PKGCONF_VERSION,
    revision = 1,
    source = Source {
        name = "pkgconf",
        Archive (
            string.gsub("https://github.com/pkgconf/pkgconf/releases/download/pkgconf-${version}/pkgconf-${version}.tar.gz", "${version}", PKGCONF_VERSION),
            "d01a1b82f0884112d08c6dfad46f43f415d070a6a5a6aaa77ebf2674e973a9b5"
        )
    },
    dependencies = {
        "base-devel",
    },
    configure = [[
        ${SOURCE_DIR}/configure --prefix=${PREFIX}
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install-strip
        install -d ${INSTALL_DIR}/${PREFIX}/share/pkgconfig/personality.d

        PERSONALITY_FILE="${INSTALL_DIR}/${PREFIX}/share/pkgconfig/personality.d/${ARCH}-unknown-fenrir.personality"
        echo "Triplet: ${ARCH}-unknown-fenrir" >> "${PERSONALITY_FILE}"
        echo "SysrootDir: ${SYSROOT_DIR}" >> "${PERSONALITY_FILE}"
        echo "DefaultSearchPaths: ${SYSROOT_DIR}/usr/lib/pkgconfig:${SYSROOT_DIR}/usr/share/pkgconfig" >> "${PERSONALITY_FILE}"
        ln -sf pkgconf "${INSTALL_DIR}/${PREFIX}/bin/${ARCH}-unknown-fenrir-pkg-config"
        ln -sf pkgconf "${INSTALL_DIR}/${PREFIX}/bin/${ARCH}-unknown-fenrir-pkgconf"
    ]]
}

return pkgconf
