//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

mod uacpi;

use core::ffi::CStr;

use uacpi_sys::{UACPI_STATUS_OK, uacpi_initialize, uacpi_status, uacpi_status_to_string};

pub fn initialize() {
    let status: uacpi_status = unsafe { uacpi_initialize(0) };
    if status != UACPI_STATUS_OK {
        log::error!("ACPI: uacpi_initialize failed: {}", unsafe {
            CStr::from_ptr(uacpi_status_to_string(status)).display()
        });
    }

    log::info!("ACPI: Initialized");
}
