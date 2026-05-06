#!/bin/bash

set -e

GIT="$1"
SOURCE_ROOT="$2"
STAMP="$3"

if [ ! -d "$SOURCE_ROOT/third_party/flanterm/.git" ]; then
    "$GIT" clone https://github.com/mintsuki/flanterm.git "$SOURCE_ROOT/third_party/flanterm" --branch=trunk --depth=1
fi

touch "$STAMP"
