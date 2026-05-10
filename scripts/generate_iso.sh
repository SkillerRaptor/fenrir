#!/bin/bash

set -e

SOURCE_ROOT="$1"
OUTPUT="$2"
XORRISO="$3"
KERNEL_BIN="$4"
KERNEL_SYMBOLS="$5"
INITRAMFS="$6"

LIMINE_DIR="$SOURCE_ROOT/third_party/limine"
ISO_ROOT="$SOURCE_ROOT/iso_root"

mkdir -p "$ISO_ROOT/boot"
cp "$KERNEL_BIN" "$ISO_ROOT/boot/"
cp "$KERNEL_SYMBOLS" "$ISO_ROOT/boot/"
cp "$INITRAMFS" "$ISO_ROOT/boot/"

mkdir -p "$ISO_ROOT/boot/limine"
cp "$SOURCE_ROOT/limine.conf" "$ISO_ROOT/boot/limine/"
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_ROOT/boot/limine/"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_ROOT/boot/limine/"
cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_ROOT/boot/limine/"

mkdir -p "$ISO_ROOT/EFI/BOOT"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_ROOT/EFI/BOOT/"
cp "$LIMINE_DIR/BOOTIA32.EFI" "$ISO_ROOT/EFI/BOOT/"

"$XORRISO" -as mkisofs \
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
    "$ISO_ROOT" -o "$OUTPUT"
