#!/bin/bash

set -e

BUILD_ROOT="$1"
OUTPUT="$2"
STAGING="$3"
shift 3
MODULES=("$@")

rm -rf "$STAGING"
mkdir -p "$STAGING"

for MODULE in "${MODULES[@]}"; do
    cp "$MODULE" "$STAGING/"
done

cp "$BUILD_ROOT/../DOOM1.WAD" "$STAGING/"

tar \
    --sort=name \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --format=ustar \
    -cf "$OUTPUT" \
    -C "$STAGING" \
    .

rm -rf "$STAGING"
