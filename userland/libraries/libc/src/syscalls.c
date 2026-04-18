/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscalls.h"

__UINT64_TYPE__ write(const char *str, const __UINT64_TYPE__ len) { return syscall(SYS_WRITE, str, len); }

__UINT64_TYPE__ enumerate_devices(struct Device *devices, __UINT64_TYPE__ *count)
{
    return syscall(SYS_ENUMERATE_DEVICES, devices, count);
}

__UINT64_TYPE__ open_device(__UINT64_TYPE__ id) { return syscall(SYS_OPEN_DEVICE, id); }

__UINT64_TYPE__ close_device(__UINT64_TYPE__ id) { return syscall(SYS_CLOSE_DEVICE, id); }

__UINT64_TYPE__ map_device(__UINT64_TYPE__ id, void **ptr) { return syscall(SYS_MAP_DEVICE, id, ptr); }
