--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local limine <const> = require("host-recipes.limine.limine")
local kernel <const> = require("recipes.kernel.kernel")

local iso = Package {
    name = "iso",
    version = "0",
    revision = 1,
    source = Source {
        name = "boot",
        Local("boot/"),
    },
    dependencies = {
        "base-devel",
        "xorriso",
        limine,
        kernel,
    },
    install = [[
        root="${BUILD_DIR}/iso_root"
        limine_share="$(dirname "$(command -v limine)")/../share/limine"

        mkdir -p "${root}/boot/limine" "${root}/EFI/BOOT" "${INSTALL_DIR}"

        cp "${SYSROOT_DIR}/boot/fenrir" "${root}/boot/"
        cp "${SOURCE_DIR}/limine.conf" "${root}/boot/limine/"
        cp "${limine_share}/limine-bios.sys" \
           "${limine_share}/limine-bios-cd.bin" \
           "${limine_share}/limine-uefi-cd.bin" "${root}/boot/limine/"
        cp "${limine_share}/BOOTX64.EFI" "${root}/EFI/BOOT/"

        xorriso -as mkisofs -R -r -J \
            -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table \
            -hfsplus -apm-block-size 2048 \
            --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image \
            --protective-msdos-label "${root}" -o "${INSTALL_DIR}/fenrir.iso"
        limine bios-install "${INSTALL_DIR}/fenrir.iso"
    ]],
}

return kernel
