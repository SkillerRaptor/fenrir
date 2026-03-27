/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/core/boot.hpp"

namespace kernel::boot {

__attribute__((used, section(".limine_requests"))) volatile u64 s_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_bootloader_info_request s_bootloader_info_request {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests"))) volatile limine_firmware_type_request s_firmware_type_request {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request s_framebuffer_request {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests_start"))) volatile u64 s_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) volatile u64 s_end_marker[] = LIMINE_REQUESTS_END_MARKER;

bool is_base_revision_supported() { return LIMINE_BASE_REVISION_SUPPORTED(s_base_revision); }

const char *get_bootloader_name() { return s_bootloader_info_request.response->name; }

const char *get_bootloader_version() { return s_bootloader_info_request.response->version; }

const char *get_firmware_type()
{
    switch (s_firmware_type_request.response->firmware_type) {
    case LIMINE_FIRMWARE_TYPE_X86BIOS:
        return "X86BIOS";
    case LIMINE_FIRMWARE_TYPE_EFI32:
        return "EFI32";
    case LIMINE_FIRMWARE_TYPE_EFI64:
        return "EFI64";
    case LIMINE_FIRMWARE_TYPE_SBI:
        return "SBI";
    default:
        return nullptr;
    }
}

usize get_framebuffer_count() { return s_framebuffer_request.response->framebuffer_count; }

limine_framebuffer *get_framebuffer(const usize framebuffer)
{
    return s_framebuffer_request.response->framebuffers[framebuffer];
}

} // namespace kernel::boot
