/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <limine.h>

#include "lib/span.hpp"
#include "lib/types.hpp"

namespace boot {

bool is_base_revision_supported();

const char *get_bootloader_name();
const char *get_bootloader_version();
const char *get_firmware_type();

u64 get_executable_physical_base();
u64 get_executable_virtual_base();

Span<limine_framebuffer *> get_framebuffers();

u64 get_hhdm_offset();

limine_mp_response *get_mp_response();

Span<limine_memmap_entry *> get_memory_map();
Span<limine_file *> get_modules();

void *get_rsdp_address();

u64 get_tsc_frequency();

} // namespace boot
