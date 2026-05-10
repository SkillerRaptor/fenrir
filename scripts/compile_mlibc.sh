#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"

SYSROOT_DIR="$SOURCE_ROOT/sysroot"
BUILD_DIR="$SOURCE_ROOT/third_party/mlibc/build"

export PATH="$SOURCE_ROOT/toolchain/usr/bin:$PATH"

cd "$SOURCE_ROOT/third_party/mlibc"

meson setup \
    --cross-file="$SOURCE_ROOT/toolchains/x86_64-fenrir.ini" \
    --prefix=/usr \
    -Ddefault_library=static \
    -Dno_headers=true \
    "$BUILD_DIR"

ninja -C "$BUILD_DIR"

DESTDIR="$SYSROOT_DIR" ninja -C "$BUILD_DIR" install

touch "$STAMP"
