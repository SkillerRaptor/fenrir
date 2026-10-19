#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

PROJECT_DIR="$(realpath ../)"
BUILD_DIR="${PROJECT_DIR}/build"
STAGE_DIR="${BUILD_DIR}/initramfs-${ARCH}"
ISO_DIR="${BUILD_DIR}/iso"

PACKAGES=(
    doomgeneric
    hello_world
)

cd ${PROJECT_DIR}

${CHARIOT} install --arch ${ARCH} --allow-new-profiles --force "${PACKAGES[@]}" "${STAGE_DIR}"

mkdir -p ${ISO_DIR}/boot

tar --sort=name \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --format=ustar \
    -C "${STAGE_DIR}" \
    -cf "${ISO_DIR}/boot/initramfs.tar" .
