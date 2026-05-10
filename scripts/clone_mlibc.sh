#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
GIT="$3"

COMMIT="362d89d"

if [ ! -d "$SOURCE_ROOT/third_party/mlibc" ]; then
    "$GIT" clone https://github.com/managarm/mlibc.git "$SOURCE_ROOT/third_party/mlibc" --branch=master
    cd "$SOURCE_ROOT/third_party/mlibc"
    git checkout "$COMMIT"
    patch -d "$SOURCE_ROOT/third_party/mlibc" -p1 < "$SOURCE_ROOT/patches/mlibc.patch"
fi

touch "$STAMP"
