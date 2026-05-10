#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
GIT="$3"

COMMIT="dfa1922c0749faf64c4423a81b233a20c08a7db7"

if [ ! -d "$SOURCE_ROOT/third_party/flanterm" ]; then
    "$GIT" clone https://github.com/mintsuki/flanterm.git "$SOURCE_ROOT/third_party/flanterm" --branch=trunk
    cd "$SOURCE_ROOT/third_party/flanterm"
    git checkout "$COMMIT"
fi

touch "$STAMP"
