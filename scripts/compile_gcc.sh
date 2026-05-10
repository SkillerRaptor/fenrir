#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
WGET="$3"

SYSROOT_DIR="$SOURCE_ROOT/sysroot"
TOOLCHAIN_DIR="$SOURCE_ROOT/toolchain"
BUILD_DIR="$TOOLCHAIN_DIR/gcc"

export PATH="$SOURCE_ROOT/toolchain/usr/bin:$PATH"

mkdir -p $TOOLCHAIN_DIR
if [ ! -d "$SOURCE_ROOT/toolchain/gcc" ]; then
    "$WGET" -qO- "https://mirrors.ocf.berkeley.edu/gnu/gcc/gcc-16.1.0/gcc-16.1.0.tar.gz" | tar -xz -C "$TOOLCHAIN_DIR" --transform 's/^gcc-16.1.0/gcc/'
fi

if [ ! -d "$BUILD_DIR/build" ]; then
    cd "$BUILD_DIR"
    patch -p1 < "$SOURCE_ROOT/patches/gcc.patch"

    mkdir build && cd build
    CFLAGS_FOR_TARGET="-march=x86-64 -mabi=sysv" \
        CXXFLAGS_FOR_TARGET="-march=x86-64 -mabi=sysv" \
        ../configure \
        --target=x86_64-fenrir \
        --prefix=/usr \
        --with-sysroot="$SYSROOT_DIR" \
        --enable-languages=c,c++ \
        --enable-threads=posix \
        --disable-multilib \
        --enable-shared \
        --enable-host-shared

    make -j$(nproc) all-gcc all-target-libgcc

    DESTDIR="$TOOLCHAIN_DIR" make install-gcc install-target-libgcc
fi

touch "$STAMP"
