#!/bin/bash

set -e

SOURCE_ROOT="$1"
BUILD_ROOT="$2"
STAMP="$3"
GIT="$4"
WGET="$5"
COMMIT="$6"

REPOSITORY_DIR="$SOURCE_ROOT/userland/ports/doomgeneric"

if [ ! -d "$REPOSITORY_DIR" ]; then
    "$GIT" clone https://github.com/ozkl/doomgeneric "$REPOSITORY_DIR" --branch=master
    cd "$REPOSITORY_DIR"
    "$GIT" checkout "$COMMIT"
    patch -d "$REPOSITORY_DIR" -p1 < "$SOURCE_ROOT/patches/doomgeneric.patch"
    "$WGET" -qO "$BUILD_ROOT/DOOM1.WAD" "https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad"
else
    cd "$REPOSITORY_DIR"
    CURRENT_COMMIT=$("$GIT" rev-parse HEAD)
    if [ "$SOURCE_ROOT/patches/doomgeneric.patch" -nt "$BUILD_ROOT/$STAMP" ] || [ "$CURRENT_COMMIT" != "$COMMIT" ]; then
        "$GIT" clean -fd
        "$GIT" reset --hard
        "$GIT" fetch origin
        "$GIT" checkout "$COMMIT"
        patch -d "$REPOSITORY_DIR" -p1 < "$SOURCE_ROOT/patches/doomgeneric.patch"
    fi
fi

touch "$STAMP"
