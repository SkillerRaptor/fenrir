#!/bin/bash

set -e

OUTPUT="$1"
STAGING="$2"
shift 2
MODULES=("$@")

rm -rf "$STAGING"
mkdir -p "$STAGING"

for MODULE in "${MODULES[@]}"; do
    cp "$MODULE" "$STAGING/"
done

tar \
    --sort=name \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --format=ustar \
    -cf "$OUTPUT" \
    -C "$STAGING" \
    .

rm -rf "$BUILD_ROOT/staging"
