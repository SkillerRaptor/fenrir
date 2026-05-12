#!/bin/bash

set -e

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
STAMP="$3"

BUILD_DIR="$BUILD_ROOT/userland/"

export PATH="$SOURCE_ROOT/toolchain/usr/bin:$PATH"

if [ "$BUILD_ROOT/doomgeneric.stamp" -nt "$STAMP" ] || [ "$BUILD_ROOT/mlibc.stamp" -nt "$STAMP" ]; then
    rm -rf $BUILD_DIR
fi

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    meson setup \
        --cross-file="$SOURCE_ROOT/toolchains/x86_64-fenrir.ini" \
        --prefix=/usr \
        "$BUILD_DIR" \
        "$SOURCE_ROOT/userland"
fi

meson compile -C "$BUILD_DIR"
DESTDIR="$SOURCE_ROOT/sysroot" meson install -C "$BUILD_DIR"

touch "$STAMP"
