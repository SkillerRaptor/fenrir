#!/bin/bash

set -e

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
STAMP="$3"
GIT="$4"
COMMIT="$5"

REPOSITORY_DIR="$SOURCE_ROOT/toolchain/mlibc"
BUILD_DIR="$REPOSITORY_DIR/headers-build"
SYSROOT_DIR="$SOURCE_ROOT/sysroot"

INSTALL=false

if [ ! -d "$REPOSITORY_DIR" ]; then
    "$GIT" clone https://github.com/managarm/mlibc.git "$REPOSITORY_DIR" --branch=master
    cd "$REPOSITORY_DIR"
    git checkout "$COMMIT"
    patch -d "$REPOSITORY_DIR" -p1 < "$SOURCE_ROOT/patches/mlibc.patch"
    INSTALL=true
else
    cd "$REPOSITORY_DIR"
    CURRENT_COMMIT=$("$GIT" rev-parse HEAD)
    if [ "$SOURCE_ROOT/patches/mlibc.patch" -nt "$BUILD_ROOT/$STAMP" ] || [ "$CURRENT_COMMIT" != "$COMMIT" ]; then
        "$GIT" reset --hard
        "$GIT" clean -fd
        "$GIT" fetch origin
        "$GIT" checkout "$COMMIT"
        patch -d "$REPOSITORY_DIR" -p1 < "$SOURCE_ROOT/patches/mlibc.patch"
        INSTALL=true
    fi
fi

if [ "$INSTALL" = true ] || [ ! -d "$BUILD_DIR" ]; then
    rm -rf $BUILD_DIR
    cd "$REPOSITORY_DIR"

    meson setup \
        --cross-file="$SOURCE_ROOT/toolchains/x86_64-fenrir.ini" \
        --prefix=/usr \
        -Dheaders_only=true \
        "$BUILD_DIR"

    mkdir -p "$SYSROOT_DIR"
    DESTDIR="$SYSROOT_DIR" ninja -C "$BUILD_DIR" install

    cd "$BUILD_ROOT"
    touch "$STAMP"
fi

