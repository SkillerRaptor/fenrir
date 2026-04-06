/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "core/boot.hpp"

namespace kernel::boot {

__attribute__((used, section(".limine_requests"))) volatile u64 s_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_bootloader_info_request s_bootloader_info_request {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((
    used, section(".limine_requests"))) volatile limine_executable_address_request s_executable_address_request {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
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

__attribute__((used, section(".limine_requests"))) volatile limine_hhdm_request s_hhdm_request {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests"))) volatile limine_mp_request s_mp_request {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
    .flags = 0,
};

__attribute__((used, section(".limine_requests"))) volatile limine_memmap_request s_memmap_request {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests"))) limine_internal_module s_kernel_symbols_module {
    .path = "kernel_symbols.map",
    .string = "kernel_symbols",
    .flags = LIMINE_INTERNAL_MODULE_REQUIRED,
};

__attribute__((used, section(".limine_requests"))) limine_internal_module *s_internal_modules[] {
    &s_kernel_symbols_module,
};

__attribute__((used, section(".limine_requests"))) volatile limine_module_request s_module_request {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 1,
    .response = nullptr,
    .internal_module_count = 1,
    .internal_modules = s_internal_modules,
};

__attribute__((used, section(".limine_requests"))) volatile limine_rsdp_request s_rsdp_request {
    .id = LIMINE_RSDP_REQUEST_ID,
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

u64 get_executable_physical_base() { return s_executable_address_request.response->physical_base; }

u64 get_executable_virtual_base() { return s_executable_address_request.response->virtual_base; }

usize get_framebuffer_count() { return s_framebuffer_request.response->framebuffer_count; }

limine_framebuffer *get_framebuffer(const usize index) { return s_framebuffer_request.response->framebuffers[index]; }

u64 get_hhdm_offset() { return s_hhdm_request.response->offset; }

limine_mp_response *get_mp_response() { return s_mp_request.response; }

usize get_memory_map_entry_count() { return s_memmap_request.response->entry_count; }

limine_memmap_entry *get_memory_map_entry(const usize index) { return s_memmap_request.response->entries[index]; }

limine_module_response *get_module_response() { return s_module_request.response; }

void *get_rsdp_address() { return s_rsdp_request.response->address; }

} // namespace kernel::boot
