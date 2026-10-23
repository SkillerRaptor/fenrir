#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname -- "$0")"

CHARIOT_URL="https://github.com/chariot-build/chariot/releases/download/2026-10-14/chariot-linux-x86_64"
CHARIOT_HASH="8ff60b602af522bbdfa95af9c760d84bf984812d581b5b27427742b526f24599"

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

PROJECT_DIR="$(realpath ../)"
BUILD_DIR="${PROJECT_DIR}/build"
PACKAGES_DIR="${BUILD_DIR}/host-packages"

HOST_PACKAGES=(
    limine
)

mkdir -p "${CHARIOT_DIR}"

echo "Downloading Chariot"
wget -q --show-progress "${CHARIOT_URL}" -O "${CHARIOT}"

actual_hash="$(sha256sum "${CHARIOT}" | cut -d' ' -f1)"
if [ "${CHARIOT_HASH}" != "${actual_hash}" ]; then
    echo "Chariot hash does not match. Expected ${CHARIOT_HASH}, got ${actual_hash}"
    exit 1
fi

chmod +x "${CHARIOT}"

cd "${PROJECT_DIR}"

echo "Building Host Packages: ${HOST_PACKAGES[@]}"
${CHARIOT} build --arch x86_64 --allow-new-profiles --tool "${HOST_PACKAGES[@]}"

for pkg in "${HOST_PACKAGES[@]}"; do
    echo "Installing: ${pkg}"
    ${CHARIOT} install --arch x86_64 --allow-new-profiles --tool --force "${pkg}" "${PACKAGES_DIR}/${pkg}"
done

echo "Finished bootstrap"
