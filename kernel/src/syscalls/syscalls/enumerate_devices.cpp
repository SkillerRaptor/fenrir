/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "core/boot.hpp"
#include "syscall/syscalls.hpp"

namespace syscalls {

struct Device {
    const char *name;
    u64 id;

    u64 width;
    u64 height;
    u64 pitch;
};

u64 sys$enumerate_devices(const ygg::Span<const u64> arguments)
{
    Device *devices = reinterpret_cast<Device *>(arguments[0]);
    u64 *count = reinterpret_cast<u64 *>(arguments[1]);

    if (!devices) {
        *count = 1;
        return 0;
    }

    devices[0] = {
        .name = "Framebuffer",
        .id = 1,
        .width = boot::get_framebuffers()[0]->width,
        .height = boot::get_framebuffers()[0]->height,
        .pitch = boot::get_framebuffers()[0]->pitch,
    };
    *count = 1;

    return 0;
}

} // namespace syscalls
