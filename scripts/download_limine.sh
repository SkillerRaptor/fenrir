#!/bin/bash

set -e

CURL="$1"
SOURCE_ROOT="$2"
STAMP="$3"

if [ ! -d "$SOURCE_ROOT/third_party/limine" ]; then
    mkdir -p "$SOURCE_ROOT/third_party"

    TAG=$("$CURL" -s https://api.github.com/repos/limine-bootloader/limine/releases/latest | grep '"tag_name"' | cut -d'"' -f4)
    "$CURL" -L "https://github.com/limine-bootloader/limine/releases/download/$TAG/limine-binary.tar.gz" | tar -xz -C "$SOURCE_ROOT/third_party" --transform 's/^limine-binary/limine/'

    make -C "$SOURCE_ROOT/third_party/limine"
fi

touch "$STAMP"
