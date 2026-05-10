#!/bin/bash

set -e

CARGO="$1"
SOURCE_ROOT="$2"
BUILD_ROOT="$3"

"$CARGO" clean --manifest-path "$SOURCE_ROOT/Cargo.toml"

rm -rf \
    "$SOURCE_ROOT/iso_root" \
    "$SOURCE_ROOT/sysroot" \
    "$SOURCE_ROOT/target" \
    "$SOURCE_ROOT/third_party" \
    "$SOURCE_ROOT/toolchain"

find "$BUILD_ROOT" -name '*.stamp' -delete

if [ -d "$BUILD_ROOT/userland/" ]; then
    cd $BUILD_ROOT/userland
    meson compile --clean
fi
