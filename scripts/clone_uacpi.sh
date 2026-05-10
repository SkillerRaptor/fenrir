#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
GIT="$3"
COMMIT="$4"

REPOSITORY_DIR="$SOURCE_ROOT/third_party/uacpi"

if [ ! -d "$REPOSITORY_DIR" ]; then
    "$GIT" clone https://github.com/uACPI/uACPI "$REPOSITORY_DIR" --branch=master
    cd "$REPOSITORY_DIR"
    "$GIT" checkout "$COMMIT"
    touch "$STAMP"
else
    cd "$REPOSITORY_DIR"
    CURRENT_COMMIT=$("$GIT" rev-parse HEAD)
    if [ "$CURRENT_COMMIT" != "$COMMIT" ]; then
        "$GIT" fetch origin
        "$GIT" checkout "$COMMIT"
        touch "$STAMP"
    fi
fi
