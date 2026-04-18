/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

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

typedef enum : uint64_t {
    STATUS_OK = 0,
} Status;

// TODO: Maybe add a separate return type?
extern Status syscall(uint64_t number, ...);

Status write(const char *str, uint64_t len);

struct Device {
    const char *name;
    uint64_t id;

    // FIXME: This is framebuffer specific and should be requested separately
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

Status enumerate_devices(struct Device *devices, uint64_t *count);
Status open_device(uint64_t id);
Status close_device(uint64_t id);
Status map_device(uint64_t id, void **ptr);

#ifdef __cplusplus
}
#endif
