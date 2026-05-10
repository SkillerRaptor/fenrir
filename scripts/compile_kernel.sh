#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
CARGO="$3"
KERNEL_BIN="$4"
KERNEL_SYMBOLS="$5"

shift 5

CARGO_FLAGS=("$@")

CARGO_TERM_COLOR=always \
    RUSTFLAGS="-C relocation-model=static" \
    "$CARGO" build --bin kernel "${CARGO_FLAGS[@]}" --manifest-path "$SOURCE_ROOT/Cargo.toml"
nm -Cn "$KERNEL_BIN" | grep -e ' t ' -e ' T ' | cut -d' ' -f1,3- > "$KERNEL_SYMBOLS"

touch "$STAMP"
