#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
CURL="$3"
WGET="$4"

if [ ! -d "$SOURCE_ROOT/third_party/limine" ]; then
    TAG=$("$CURL" -s https://api.github.com/repos/limine-bootloader/limine/releases/latest | grep '"tag_name"' | cut -d'"' -f4)
    "$WGET" -qO- "https://github.com/limine-bootloader/limine/releases/download/$TAG/limine-binary.tar.gz" | tar -xz -C "$SOURCE_ROOT/third_party" --transform 's/^limine-binary/limine/'
    make -C "$SOURCE_ROOT/third_party/limine"
fi

touch "$STAMP"
