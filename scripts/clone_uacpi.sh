#!/bin/bash

set -e

SOURCE_ROOT="$1"
STAMP="$2"
GIT="$3"

COMMIT="71f981d4ee36bf3ae85976409734000578b5249c"

if [ ! -d "$SOURCE_ROOT/third_party/uacpi" ]; then
    "$GIT" clone https://github.com/uACPI/uACPI "$SOURCE_ROOT/third_party/uacpi" --branch=master
    cd "$SOURCE_ROOT/third_party/uacpi"
    git checkout "$COMMIT"
fi

touch "$STAMP"
