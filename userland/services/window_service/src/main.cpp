/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <syscalls.h>

int main()
{
    __UINT64_TYPE__ count = 0;
    enumerate_devices(nullptr, &count);

    // TODO: Add heap allocation and create from count
    Device device[16];
    enumerate_devices(&device[0], &count);

    open_device(device[0].id);
    void *ptr = nullptr;
    map_device(device[0].id, &ptr);

    __UINT32_TYPE__ *framebuffer = static_cast<__UINT32_TYPE__ *>(ptr);
    for (__UINT64_TYPE__ y = 0; y < device[0].height; y++) {
        for (__UINT64_TYPE__ x = 0; x < device[0].width; x++) {
            const __UINT32_TYPE__ n_x = x * 255 / device[0].width;
            const __UINT32_TYPE__ n_y = y * 255 / device[0].height;
            framebuffer[y * (device[0].pitch / 4) + x] = (n_y << 8) | n_x;
        }
    }

    close_device(device[0].id);

    return 0;
}
