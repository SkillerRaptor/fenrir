/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <limine.h>

#include "lib/types.hpp"

namespace boot {

bool is_base_revision_supported();

const char *get_bootloader_name();
const char *get_bootloader_version();
const char *get_firmware_type();

u64 get_executable_physical_base();
u64 get_executable_virtual_base();

usize get_framebuffer_count();
limine_framebuffer *get_framebuffer(usize index);

u64 get_hhdm_offset();

limine_mp_response *get_mp_response();

usize get_memory_map_entry_count();
limine_memmap_entry *get_memory_map_entry(usize index);

limine_module_response *get_module_response();

void *get_rsdp_address();

} // namespace boot
