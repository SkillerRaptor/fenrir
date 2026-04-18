/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <leviathan/syscall.h>

int main()
{
    uint64_t count = 0;
    enumerate_devices(nullptr, &count);

    // TODO: Add heap allocation and create from count
    Device devices[16];
    enumerate_devices(devices, &count);

    open_device(devices[0].id);
    void *ptr = nullptr;
    map_device(devices[0].id, &ptr);

    uint32_t *framebuffer = static_cast<uint32_t *>(ptr);
    for (uint64_t y = 0; y < devices[0].height; y++) {
        for (uint64_t x = 0; x < devices[0].width; x++) {
            const uint32_t n_x = x * 255 / devices[0].width;
            const uint32_t n_y = y * 255 / devices[0].height;
            framebuffer[y * (devices[0].pitch / 4) + x] = (n_y << 8) | n_x;
        }
    }

    close_device(devices[0].id);

    while (true) {
        asm volatile("");
    }

    return 0;
}
