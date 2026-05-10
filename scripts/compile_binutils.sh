#!/bin/bash

set -e

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
STAMP="$3"
WGET="$4"

SYSROOT_DIR="$SOURCE_ROOT/sysroot"
TOOLCHAIN_DIR="$SOURCE_ROOT/toolchain"
BUILD_DIR="$TOOLCHAIN_DIR/binutils"

mkdir -p $TOOLCHAIN_DIR
if [ ! -d "$SOURCE_ROOT/toolchain/binutils" ]; then
    "$WGET" -qO- "https://mirrors.ocf.berkeley.edu/gnu/binutils/binutils-2.46.0.tar.gz" | tar -xz -C "$TOOLCHAIN_DIR" --transform 's/^binutils-2.46.0/binutils/'
fi

if [ ! -d "$BUILD_DIR/build" ]; then
    cd "$BUILD_DIR"
    patch -p1 < "$SOURCE_ROOT/patches/binutils.patch"

    mkdir build && cd build
    ../configure \
        --target=x86_64-fenrir \
        --prefix=/usr \
        --with-sysroot="$SYSROOT_DIR" \
        --disable-werror \
        --enable-default-execstack=no

    make -j$(nproc)
    DESTDIR="$TOOLCHAIN_DIR" make install

    cd "$BUILD_ROOT"
    touch "$STAMP"
fi
