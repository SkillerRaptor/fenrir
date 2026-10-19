#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

PACKAGES_DIR="$(realpath ./host-packages)"
ISO_DIR="$(realpath ./iso/)"

mkdir -p ${ISO_DIR}/boot/limine
cp ${ISO_DIR}/../../limine.conf ${ISO_DIR}/boot/limine

cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-bios-cd.bin ${ISO_DIR}/boot/limine
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-uefi-cd.bin ${ISO_DIR}/boot/limine
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-bios.sys ${ISO_DIR}/boot/limine

mkdir -p ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTX64.EFI ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTIA32.EFI ${ISO_DIR}/EFI/BOOT/

xorriso -as mkisofs \
    -R -r -J \
    -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    -hfsplus \
    -apm-block-size 2048 \
    --efi-boot boot/limine/limine-uefi-cd.bin \
    -efi-boot-part \
    --efi-boot-image \
    --protective-msdos-label \
    ${ISO_DIR} -o ../fenrir.iso

${PACKAGES_DIR}/limine/usr/local/bin/limine bios-install ../fenrir.iso
