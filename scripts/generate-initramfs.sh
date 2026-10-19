#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

STAGE_DIR="$(realpath ./initramfs-${ARCH})"
PROJECT_DIR="$(realpath ../)"
ISO_DIR="$(realpath ./iso/)"

mkdir -p ${ISO_DIR}/boot

PACKAGES=(
    doomgeneric
    hello_world
)

cd ${PROJECT_DIR}

${CHARIOT} install --arch ${ARCH} --allow-new-profiles --force "${PACKAGES[@]}" "${STAGE_DIR}"

tar --sort=name \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --format=ustar \
    -C "${STAGE_DIR}" \
    -cf "${ISO_DIR}/boot/initramfs.tar" .
