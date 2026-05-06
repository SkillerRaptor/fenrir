#!/bin/bash

set -e

GIT="$1"
SOURCE_ROOT="$2"
STAMP="$3"

if [ ! -d "$SOURCE_ROOT/third_party/uacpi/.git" ]; then
    "$GIT" clone https://github.com/uACPI/uACPI "$SOURCE_ROOT/third_party/uacpi" --branch=master --depth=1
fi

touch "$STAMP"
