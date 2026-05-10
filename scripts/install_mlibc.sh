#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"

BUILD_DIR="$SOURCE_ROOT/third_party/mlibc/headers-build"
SYSROOT_DIR="$SOURCE_ROOT/sysroot"

if [ ! -d "$BUILD_DIR" ]; then
    cd "$SOURCE_ROOT/third_party/mlibc"

    meson setup \
        --cross-file="$SOURCE_ROOT/toolchains/x86_64-fenrir.ini" \
        --prefix=/usr \
        -Dheaders_only=true \
        "$BUILD_DIR"

    mkdir -p "$SYSROOT_DIR"
    DESTDIR="$SYSROOT_DIR" ninja -C "$BUILD_DIR" install
fi

touch "$STAMP"
