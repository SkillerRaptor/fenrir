#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname -- "$0")"

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

PROJECT_DIR="$(realpath ../)"

cd ${PROJECT_DIR}

PACKAGE="${PACKAGE:-kernel}"
SOURCE="${SOURCE:-${PACKAGE}}"
LOCAL_DIR="${LOCAL_DIR:-${PROJECT_DIR}/${PACKAGE}}"

exec ${CHARIOT} lsp --allow-new-profiles --arch x86_64 -m "${LOCAL_DIR}=${SOURCE}" clangd "${PACKAGE}"
