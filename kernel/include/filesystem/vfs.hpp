/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/boot.hpp"
#include "lib/types.hpp"

namespace vfs {

struct File;

enum class SeekOrigin {
    Current,
    Set,
    End
};

// FIXME: Using `limine_file` here is a HACK and should be replaced with a driver
void mount(limine_file *archive, const char *target, const char *filesystem_type);
// TODO: Add a way to unmount a driver and not from the target
void unmount(const char *target);

File *open(const char *path);
void close(File *);

// FIXME: Implement write
void read(File *, u8 *buffer, usize size);

void seek(File *, usize offset, SeekOrigin);
usize tell(File *);

} // namespace vfs

/*
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
mountpoint_t *add_mountpoint(mountpoint_t *mountpoint);
void remove_mountpoint(mountpoint_t *mountpoint);
mountpoint_t get_mountpoint(char *path);
mountpoint_t *get_mountpoint_by_id(size_t mountpoint_id);


int ustar_open(char *path, int flags);
int ustar_close(int ustar_fd);
void ustar_read(int ustar_fd, void *buffer, size_t count);
int ustar_close(int ustar_fd);

int open(const char *filename, int flags)
{
    mountpoint_t *mountpoint = get_mountpoint(pathname);

    if (mountpoint != NULL) {
        char *rel_path = get_rel_path(mountpoint, path);
        int fs_specific_id = mountpoint->operations->open(rel_path, flags);
        if (fs_specific_id != ERROR) {
            // IMPLEMENTATION LEFT AS EXERCISE
// Get a new vfs descriptor id vfs_id
vfs_opened_files[vfs_id] = // fill the file descriptor entry at position

    resource_t *res = // left as an exercise, see below

    // Open the resource and return its id to the calling process
    int handle = register_resource(current_process, res);
return handle;
}
}
return ERROR;
}
int close(resource_t *res)
{
    // Your code should check that these actually exist
    file_descriptor_t *file = res->impl;
    mountpoint_t *mountpoint = get_mountpoint_by_id(file.mountpoint_id);

    if (mountpoint->operations->close) {
        mountpoint->operations->close(file->fs_file_id);
    }

    free(f);
    remove_resource(current_process, res);

    return 0;
}
int vfs_mount(const char *device, const char *target, const char *fs_type);
int vfs_umount(const char *device, const char *target);

ssize_t read(resource_t *res, void *buf, size_t nbytes)
{
    // Your code should check these actually exist
    file_descriptor_t *file = res->impl;
    mountpoint_t *mountpoint = get_mountpoint_by_id(file.mountpoint_id);

    int fs_file_id = file.fs_file_id;
    int bytes_read = mountpoint->operations->read(fs_file_id, buf, nbytes);

    if (file.buf_read_pos + nbytes < file.file_size) {
        file.buf_read_pos += nbytes;
    } else {
        file.buf_read_pos = file.file_size;
    }
    return bytes_read;
}

typedef struct {
    uint64_t fs_file_id;
    int mountpoint_id;
    char *filename;
    int buf_read_pos;
    int buf_write_pos;
    int file_size;
    char *file_buffer;
} file_descriptor_t;

int file_descriptor;
char *buffer = calloc(11, sizeof(char));
if (buffer == NULL) {
    return -1;
}
int file_descriptor = open("/path/to/file/to_open", O_RDONLY);
int sz = read(file_descriptor, buffer, 10);
buffer[sz] = '\0';
printf("%s", buffer);
close(file_descriptor);
*/
