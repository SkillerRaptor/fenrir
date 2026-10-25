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

const char *boot_get_firmware_type();

const struct limine_framebuffer_response *boot_get_framebuffer_response();
