#!/bin/bash

set -e

CARGO="$1"
SOURCE_ROOT="$2"

"$CARGO" clean --manifest-path "$SOURCE_ROOT/Cargo.toml"

rm -rf \
    "$SOURCE_ROOT/third_party" \
    "$SOURCE_ROOT/iso_root" \
    "$SOURCE_ROOT/target"
