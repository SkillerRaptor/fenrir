//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use limine::{
    BaseRevision,
    RequestsEndMarker,
    RequestsStartMarker,
    firmware::{
        FIRMWARE_TYPE_EFI32,
        FIRMWARE_TYPE_EFI64,
        FIRMWARE_TYPE_SBI,
        FIRMWARE_TYPE_X86BIOS,
    },
    framebuffer::Framebuffer,
    memmap::Entry,
    request::{
        BootloaderInfoRequest,
        ExecutableAddressRequest,
        FirmwareTypeRequest,
        FramebufferRequest,
        HhdmRequest,
        MemmapRequest,
    },
};

#[used]
#[unsafe(link_section = ".limine_requests")]
static BASE_REVISION: BaseRevision = BaseRevision::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static BOOTLOADER_INFO_REQUEST: BootloaderInfoRequest = BootloaderInfoRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static EXECUTABLE_ADDRESS_REQUEST: ExecutableAddressRequest = ExecutableAddressRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static FIRMWARE_TYPE_REQUEST: FirmwareTypeRequest = FirmwareTypeRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static FRAMEBUFFER_REQUEST: FramebufferRequest = FramebufferRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static HHDM_REQUEST: HhdmRequest = HhdmRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static MEMORY_MAP_REQUEST: MemmapRequest = MemmapRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests_start")]
static _START_MARKER: RequestsStartMarker = RequestsStartMarker::new();

#[used]
#[unsafe(link_section = ".limine_requests_end")]
static _END_MARKER: RequestsEndMarker = RequestsEndMarker::new();

pub fn is_base_revision_supported() -> bool {
    BASE_REVISION.is_supported()
}

pub fn get_bootloader_name() -> &'static str {
    BOOTLOADER_INFO_REQUEST.response().unwrap().name()
}

pub fn get_bootloader_version() -> &'static str {
    BOOTLOADER_INFO_REQUEST.response().unwrap().version()
}

pub fn get_executable_physical_base() -> u64 {
    EXECUTABLE_ADDRESS_REQUEST.response().unwrap().physical_base
}

pub fn get_executable_virtual_base() -> u64 {
    EXECUTABLE_ADDRESS_REQUEST.response().unwrap().virtual_base
}

pub fn get_firmware_type() -> &'static str {
    match FIRMWARE_TYPE_REQUEST.response().unwrap().firmware_type {
        FIRMWARE_TYPE_X86BIOS => "X86BIOS",
        FIRMWARE_TYPE_EFI32 => "EFI32",
        FIRMWARE_TYPE_EFI64 => "EFI64",
        FIRMWARE_TYPE_SBI => "SBI",
        _ => unreachable!(),
    }
}

pub fn get_framebuffers() -> &'static [&'static Framebuffer] {
    FRAMEBUFFER_REQUEST.response().unwrap().framebuffers()
}

pub fn get_hhdm_offset() -> u64 {
    HHDM_REQUEST.response().unwrap().offset
}

pub fn get_memory_map() -> &'static [&'static Entry] {
    MEMORY_MAP_REQUEST.response().unwrap().entries()
}
