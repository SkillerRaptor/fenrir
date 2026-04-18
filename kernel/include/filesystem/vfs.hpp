/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/boot.hpp"
#include "lib/string_view.hpp"
#include "lib/types.hpp"

namespace vfs {

struct File;

enum class SeekOrigin {
    Current,
    Set,
    End
};

// FIXME: Using `limine_file` here is a HACK and should be replaced with a driver
void mount(limine_file *archive, StringView target, const char *filesystem_type);
// TODO: Add a way to unmount a driver and not from the target
void unmount(StringView target);

File *open(StringView path);
void close(File *);

// FIXME: Implement write
void read(File *, u8 *buffer, usize size);

void seek(File *, usize offset, SeekOrigin);
usize tell(File *);

} // namespace vfs
