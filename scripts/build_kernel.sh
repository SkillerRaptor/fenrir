#!/bin/bash

set -e

CARGO="$1"
SOURCE_ROOT="$2"
STAMP="$3"
KERNEL_BIN="$4"
KERNEL_SYMBOLS="$5"

shift 5

CARGO_FLAGS=("$@")

export CARGO_TERM_COLOR=always
export RUSTFLAGS="-C relocation-model=static"

"$CARGO" build --bin kernel "${CARGO_FLAGS[@]}" --manifest-path "$SOURCE_ROOT/Cargo.toml"

if [ ! -f "$STAMP" ] || [ "$KERNEL_BIN" -nt "$STAMP" ]; then
    nm -Cn "$KERNEL_BIN" | grep -e ' t ' -e ' T ' | cut -d' ' -f1,3- > "$KERNEL_SYMBOLS"
    touch "$STAMP"
fi
