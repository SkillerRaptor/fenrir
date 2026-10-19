#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

PROJECT_DIR="$(realpath ../)"
BUILD_DIR="${PROJECT_DIR}/build"
PACKAGES_DIR="${BUILD_DIR}/host-packages"
ISO_DIR="${BUILD_DIR}/iso"
KERNEL_BUILD_DIR="${BUILD_DIR}/kernel"
KERNEL_DIR="$(realpath ../kernel)"

cd ${KERNEL_DIR}

CARGO_TARGET_DIR="${KERNEL_BUILD_DIR}" \
RUSTFLAGS="-C relocation-model=static" \
cargo build --bin kernel

mkdir -p ${ISO_DIR}/boot/

cp "${KERNEL_BUILD_DIR}/${ARCH}-fenrir/debug/kernel" "${ISO_DIR}/boot/kernel"
nm -Cn "${KERNEL_BUILD_DIR}/${ARCH}-fenrir/debug/kernel" | grep -e ' t ' -e ' T ' | cut -d' ' -f1,3- > "${ISO_DIR}/boot/kernel_symbols.map"
