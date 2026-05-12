#!/bin/bash

set -e

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
STAMP="$3"

REPOSITORY_DIR="$SOURCE_ROOT/toolchain/mlibc"
BUILD_DIR="$REPOSITORY_DIR/build"
SYSROOT_DIR="$SOURCE_ROOT/sysroot"

export PATH="$SOURCE_ROOT/toolchain/usr/bin:$PATH"

if [ ! -f "$STAMP" ]; then
    cd "$REPOSITORY_DIR"

    meson setup \
        --cross-file="$SOURCE_ROOT/toolchains/x86_64-fenrir.ini" \
        --prefix=/usr \
        -Ddefault_library=static \
        -Dno_headers=true \
        "$BUILD_DIR"
fi

if [ "$BUILD_ROOT/mlibc_install.stamp" -nt "$STAMP" ]; then
    ninja -C "$BUILD_DIR"

    DESTDIR="$SYSROOT_DIR" ninja -C "$BUILD_DIR" install

    cd "$BUILD_ROOT"
    touch "$STAMP"
fi

