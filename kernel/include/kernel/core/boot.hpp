/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <limine.h>

#include "kernel/core/types.hpp"

namespace kernel::boot {

bool is_base_revision_supported();

const char *get_bootloader_name();
const char *get_bootloader_version();
const char *get_firmware_type();

usize get_framebuffer_count();
limine_framebuffer *get_framebuffer(usize index);

u64 get_hhdm_offset();

usize get_memory_map_entry_count();
limine_memmap_entry *get_memory_map_entry(usize index);

} // namespace kernel::boot
