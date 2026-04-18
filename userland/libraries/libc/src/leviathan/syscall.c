/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "leviathan/syscall.h"

Status enumerate_devices(struct Device *devices, uint64_t *count)
{
    return syscall(SYS_ENUMERATE_DEVICES, devices, count);
}

Status open_device(uint64_t id) { return syscall(SYS_OPEN_DEVICE, id); }

Status close_device(uint64_t id) { return syscall(SYS_CLOSE_DEVICE, id); }

Status map_device(uint64_t id, void **ptr) { return syscall(SYS_MAP_DEVICE, id, ptr); }
