/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

#include <limine.h>

bool boot_is_base_revision_supported(void);

struct limine_framebuffer_response *boot_get_framebuffer_response(void);
