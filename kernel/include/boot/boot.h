/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

#include <limine.h>

bool boot_is_base_revision_supported();

const char *boot_get_bootloader_name();
const char *boot_get_bootloader_version();

uint64_t boot_get_executable_physical_base();
uint64_t boot_get_executable_virtual_base();

const char *boot_get_firmware_type();

const struct limine_framebuffer_response *boot_get_framebuffer_response();

uint64_t boot_get_hhdm_offset();

const struct limine_memmap_response *boot_get_memory_map_response();
