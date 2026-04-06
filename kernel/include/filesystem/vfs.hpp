/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace kernel::ustar {

int open(char *path, int flags);
int close(int ustar_fd);
void read(int ustar_fd, void *buffer, size_t count);

} // namespace kernel::ustar

namespace kernel::vfs {

#define VFS_TYPE_LENGTH 32
#define VFS_PATH_LENGTH 64

struct mountpoint_t {
    char type[VFS_TYPE_LENGTH];
    char device[VFS_PATH_LENGTH];
    char mountpoint[VFS_PATH_LENGTH];

    // Driver provided file system operations (open/read/write/close)
    fs_operations_t *operations;
};

mountpoint_t *create_mountpoint(...)
{
    mountpoint_t *new_mountpoint = malloc(sizeof(mountpoint_t)); // We assume a kind of malloc is present
    if (new_mountpoint == NULL) {
        return -1;
    }
    strcpy(new_mountpoint->device, device);
    strcpy(new_mountpoint->type, type);
    strcpy(new_mountpoint->mountpoint, target);
    new_mountpoint->operations = NULL;
}
mountpoint_t *add_mountpoint(mountpoint_t *);
void remove_mountpoint(mountpoint_t *);
mountpoint_t get_mountpoint(char *path);

#define MAX_MOUNTPOINTS 12
mountpoint_t *mountpoints_root;

// device = some i/o device
// target = path
// fs type = tar,ext2...

int vfs_mount(char *device, char *target, char *fs_type)
{
    // Once called it will simply create and add a new item to the mountpoints list, and will populate the data
    // structure with the information provided
}

int vfs_umount(char *device, char *target)
{
    // Uses either device or target
}

void initialize()
{
    // TODO: Create root mount point at '/'
}

} // namespace kernel::vfs
