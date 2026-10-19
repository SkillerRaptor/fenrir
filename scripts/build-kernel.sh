#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

PACKAGES_DIR="$(realpath ./host-packages)"
KERNEL_DIR="$(realpath ../kernel)"
ISO_DIR="$(realpath ./iso/)"

mkdir -p ${ISO_DIR}/boot/

cd ${KERNEL_DIR}

RUSTFLAGS="-C relocation-model=static" cargo build --bin kernel
cp "${KERNEL_DIR}/target/${ARCH}-fenrir/debug/kernel" "${ISO_DIR}/boot/kernel"
nm -Cn "${KERNEL_DIR}/target/${ARCH}-fenrir/debug/kernel" | grep -e ' t ' -e ' T ' | cut -d' ' -f1,3- > "${ISO_DIR}/boot/kernel_symbols.map"
