#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
CURL="$3"
WGET="$4"
TAG="$5"

REPOSITORY_DIR="$SOURCE_ROOT/third_party/limine"

clone_limine() {
    rm -rf "$REPOSITORY_DIR"
    "$WGET" -qO- "https://github.com/limine-bootloader/limine/releases/download/$TAG/limine-binary.tar.gz" | tar -xz -C "$SOURCE_ROOT/third_party" --transform 's/^limine-binary/limine/'
    echo "$TAG" > "$REPOSITORY_DIR/.version"
    make -C "$SOURCE_ROOT/third_party/limine"
    touch "$STAMP"
}

if [ ! -d "$SOURCE_ROOT/third_party/limine" ]; then
    clone_limine
else
    CURRENT_TAG=$(cat "$REPOSITORY_DIR/.version" 2>/dev/null || echo "")
    if [ "$CURRENT_TAG" != "$TAG" ]; then
        clone_limine
    fi
fi
