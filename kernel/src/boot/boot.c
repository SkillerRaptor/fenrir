/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "boot/boot.h"

#include "limine.h"

[[gnu::used, gnu::section(".limine_requests")]]
static volatile uint64_t s_base_revision[] = LIMINE_BASE_REVISION(6);

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_bootloader_info_request s_bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_executable_address_request s_executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_firmware_type_request s_firmware_type_request = {
    .id = LIMINE_FIRMWARE_TYPE_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_framebuffer_request s_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_hhdm_request s_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_memmap_request s_memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
static struct limine_internal_module s_symbol_map_module = {
    .path = "kernel_symbols.map",
    .string = "kernel_symbols",
    .flags = LIMINE_INTERNAL_MODULE_REQUIRED,
};

[[gnu::used, gnu::section(".limine_requests")]]
static struct limine_internal_module *s_internal_modules_ptr[] = {
    &s_symbol_map_module,
};

[[gnu::used, gnu::section(".limine_requests")]]
static volatile struct limine_module_request s_modules_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 1,
    .internal_module_count = 1,
    .internal_modules = s_internal_modules_ptr,
};

[[gnu::used, gnu::section(".limine_requests_start")]]
static volatile uint64_t s_start_marker[] = LIMINE_REQUESTS_START_MARKER;

[[gnu::used, gnu::section(".limine_requests_end")]]
static volatile uint64_t s_end_marker[] = LIMINE_REQUESTS_END_MARKER;

bool boot_is_base_revision_supported() { return LIMINE_BASE_REVISION_SUPPORTED(s_base_revision); }

const char *boot_get_bootloader_name() { return s_bootloader_info_request.response->name; }

const char *boot_get_bootloader_version() { return s_bootloader_info_request.response->version; }

uint64_t boot_get_executable_physical_base() { return s_executable_address_request.response->physical_base; }

uint64_t boot_get_executable_virtual_base() { return s_executable_address_request.response->virtual_base; }

const char *boot_get_firmware_type()
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
        return "<unknown>";
    }
}

const struct limine_framebuffer_response *boot_get_framebuffer_response() { return s_framebuffer_request.response; }

uint64_t boot_get_hhdm_offset() { return s_hhdm_request.response->offset; }

const struct limine_memmap_response *boot_get_memory_map_response() { return s_memory_map_request.response; }

const struct limine_module_response *boot_get_module_response() { return s_modules_request.response; }
