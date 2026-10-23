--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local LIMINE_VERSION <const> = "12.9.4"

local limine = Tool {
    name = "limine",
    version = LIMINE_VERSION,
    revision = 1,
    source = Source {
        name = "limine",
        Archive (
            string.gsub("https://github.com/Limine-Bootloader/Limine/releases/download/v${version}/limine-${version}.tar.gz", "${version}", LIMINE_VERSION),
            "4f75ffca8e11a1030e43e2b9ab82fac1a0f8985e1c40d429fea9fd2972693447"
        ),
        dependencies = {
            "base-devel"
        },
        prepare = [[
            cd "${SOURCE_DIR}"
            ./bootstrap
        ]],
    },
    dependencies = {
        "base-devel", "mtools", "nasm"
    },
    configure = [[
        CC_FOR_TARGET=gcc \
        LD_FOR_TARGET=ld \
        OBJCOPY_FOR_TARGET=objcopy \
        OBJDUMP_FOR_TARGET=objdump \
        READELF_FOR_TARGET=readelf \
        LDFLAGS="-static" \
        CFLAGS_FOR_TARGET="-O2 -pipe" \
        "${SOURCE_DIR}"/configure \
            --enable-bios-cd \
            --enable-bios-pxe \
            --enable-bios \
            --enable-uefi-ia32 \
            --enable-uefi-x86-64 \
            --enable-uefi-cd \
            --prefix="${PREFIX}"
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install
    ]]
}

return limine
