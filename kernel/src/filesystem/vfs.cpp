/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "filesystem/vfs.hpp"

#include "filesystem/ustar.hpp"
#include "lib/string.hpp"

namespace vfs {

struct MountPoint {
    // FIXME: Add virtual class
    // TODO: Add filesystem type

    // NOTE: This is a hack
    limine_file *archive { nullptr };
    // NOTE: Make this heap allocated and strcpy
    const char *target { nullptr };

    MountPoint *next_mount_point { nullptr };
};

struct File {
    const u8 *data { nullptr };
    usize position { 0 };
    usize size { 0 };
};

static MountPoint *s_mount_point { nullptr };

static MountPoint *get_mount_point(const StringView path)
{
    const usize path_length = path.length();

    MountPoint *longest_match = nullptr;
    usize longest_matching_path = 0;

    MountPoint *current = s_mount_point;
    while (current != nullptr) {
        const usize mount_point_length = strlen(current->target);

        const usize smallest_length = path_length > mount_point_length ? mount_point_length : path_length;

        usize matching = 0;
        // FIXME: Match till next slash and not all chars
        for (usize i { 0 }; i < smallest_length; i++) {
            if (current->target[i] == path[i]) {
                ++matching;
            }
        }

        if (matching > longest_matching_path) {
            longest_matching_path = matching;
            longest_match = current;
        }

        current = current->next_mount_point;
    }

    return longest_match;
}

void mount(limine_file *archive, const StringView target, const char *)
{
    // FIXME: Add check if mount already exists, maybe return an error? Also handle different systems

    char *mount_target = new char[target.length() + 1];
    memcpy(mount_target, target.data(), target.length());
    if (!s_mount_point) {
        s_mount_point = new MountPoint {
            .archive = archive,
            .target = mount_target,
            .next_mount_point = nullptr,
        };
    } else {
        MountPoint *mount_point = s_mount_point;
        while (mount_point->next_mount_point != nullptr) {
            mount_point = mount_point->next_mount_point;
        }

        mount_point->next_mount_point = new MountPoint {
            .archive = archive,
            .target = mount_target,
            .next_mount_point = nullptr,
        };
    }
}

void unmount(const StringView target)
{
    // FIXME: Implement me
}

File *open(const StringView path)
{
    // FIXME: Reduce path to the mounted point

    const MountPoint *mount_point = get_mount_point(path);
    const ustar::File file = ustar::lookup(
        static_cast<const u8 *>(mount_point->archive->address),
        path.data() + strlen(mount_point->target));

    return new File {
        .data = file.ptr,
        .position = 0,
        .size = file.size,
    };
}

void close(File *file) { delete file; }

void read(File *file, u8 *buffer, const usize size)
{
    // FIXME: Check against file size

    for (usize i { 0 }; i < size; i++) {
        buffer[i] = file->data[file->position + i];
    }
}

void seek(File *file, const usize offset, const SeekOrigin origin)
{
    const usize origin_offset = [&]() -> usize {
        switch (origin) {
        case SeekOrigin::Current:
            return file->position;
        case SeekOrigin::Set:
            return 0;
        case SeekOrigin::End:
            return file->size;
        default:
            return -1;
        }
    }();

    file->position = origin_offset + offset;
}

usize tell(File *file) { return file->position; }

} // namespace vfs
