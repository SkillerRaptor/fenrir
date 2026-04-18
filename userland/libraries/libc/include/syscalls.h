/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#define SYS_EXIT 0x01
#define SYS_WRITE 0x02

// FIXME: This are hacky syscalls for now
#define SYS_ENUMERATE_DEVICES 0x03
#define SYS_OPEN_DEVICE 0x04
#define SYS_CLOSE_DEVICE 0x05
#define SYS_MAP_DEVICE 0x06

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Maybe add a separate return type?
extern __UINT64_TYPE__ syscall(__UINT64_TYPE__ number, ...);

__UINT64_TYPE__ write(const char *str, __UINT64_TYPE__ len);

struct Device {
    const char *name;
    __UINT64_TYPE__ id;

    // FIXME: This is framebuffer specific and should be requested separately
    __UINT64_TYPE__ width;
    __UINT64_TYPE__ height;
    __UINT64_TYPE__ pitch;
};

__UINT64_TYPE__ enumerate_devices(struct Device *devices, __UINT64_TYPE__ *count);
__UINT64_TYPE__ open_device(__UINT64_TYPE__ id);
__UINT64_TYPE__ close_device(__UINT64_TYPE__ id);
__UINT64_TYPE__ map_device(__UINT64_TYPE__ id, void **ptr);

#ifdef __cplusplus
}
#endif
