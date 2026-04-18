/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <syscalls.h>

int main()
{
    uint64_t count = 0;
    enumerate_devices(nullptr, &count);

    // TODO: Add heap allocation and create from count
    Device device[16];
    enumerate_devices(&device[0], &count);

    open_device(device[0].id);
    void *ptr = nullptr;
    map_device(device[0].id, &ptr);

    uint32_t *framebuffer = static_cast<uint32_t *>(ptr);
    for (uint64_t y = 0; y < device[0].height; y++) {
        for (uint64_t x = 0; x < device[0].width; x++) {
            const uint32_t n_x = x * 255 / device[0].width;
            const uint32_t n_y = y * 255 / device[0].height;
            framebuffer[y * (device[0].pitch / 4) + x] = (n_y << 8) | n_x;
        }
    }

    close_device(device[0].id);

    return 0;
}
