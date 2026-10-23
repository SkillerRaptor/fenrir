/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "boot/boot.h"

__attribute__((used, section(".limine_requests"))) static volatile uint64_t s_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((
    used, section(".limine_requests"))) static volatile struct limine_framebuffer_request s_framebuffer_request
    = { .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0 };

__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t s_start_marker[]
    = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t s_end_marker[]
    = LIMINE_REQUESTS_END_MARKER;

bool boot_is_base_revision_supported(void) { return LIMINE_BASE_REVISION_SUPPORTED(s_base_revision); }

struct limine_framebuffer_response *boot_get_framebuffer_response(void) { return s_framebuffer_request.response; }
