/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <ygg/string_view.hpp>
#include <ygg/types.hpp>

#include "core/boot.hpp"

namespace vfs {

struct File;

enum class SeekOrigin {
    Current,
    Set,
    End
};

// FIXME: Using `limine_file` here is a HACK and should be replaced with a driver
void mount(limine_file *archive, ygg::StringView target, const char *filesystem_type);
// TODO: Add a way to unmount a driver and not from the target
void unmount(ygg::StringView target);

File *open(ygg::StringView path);
void close(File *);

// FIXME: Implement write
void read(File *, u8 *buffer, usize size);

void seek(File *, usize offset, SeekOrigin);
usize tell(File *);

} // namespace vfs
