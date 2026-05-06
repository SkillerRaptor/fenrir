#!/bin/bash

set -e

CARGO="$1"
SOURCE_ROOT="$2"
BUILD_ROOT="$3"

"$CARGO" clean --manifest-path "$SOURCE_ROOT/Cargo.toml"

rm -rf \
    "$SOURCE_ROOT/third_party" \
    "$SOURCE_ROOT/iso_root" \
    "$SOURCE_ROOT/target"

rm -f \
    "$BUILD_ROOT/flanterm.stamp" \
    "$BUILD_ROOT/uacpi.stamp" \
    "$BUILD_ROOT/limine.stamp" \
    "$BUILD_ROOT/kernel.stamp"
