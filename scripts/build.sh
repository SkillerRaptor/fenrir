#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname -- "$0")"

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

TARGET="${1:?usage: build.sh <kernel|iso>}"

PROJECT_DIR="$(realpath ../)"
BUILD_DIR="${PROJECT_DIR}/build"

case "$TARGET" in
    kernel|iso) ;;
    *) echo "unknown target: $TARGET" >&2; exit 1 ;;
esac

cd ${PROJECT_DIR}

${CHARIOT} install --arch x86_64 --allow-new-profiles --force "${TARGET}" "${BUILD_DIR}/${TARGET}"
