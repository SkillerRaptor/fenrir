//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::ffi::CStr;

use uacpi_sys::{
    UACPI_STATUS_OK,
    acpi_hpet,
    uacpi_status,
    uacpi_status_to_string,
    uacpi_table,
    uacpi_table_find_by_signature,
    uacpi_table_unref,
};

use crate::{
    common::{boot, mmio},
    memory::vmm::{self, Attribute},
};

const GENERAL_CAPABILITIES_REGISTER: u64 = 0x000;
const GENERAL_CONFIGURATION_REGISTER: u64 = 0x010;
const MAIN_COUNTER_REGISTER: u64 = 0x0f0;

static mut VIRTUAL_ADDRESS: u64 = 0;
static mut CLOCK_PERIOD: u32 = 0;

pub fn initialize() {
    let mut table = uacpi_table::default();
    let status: uacpi_status =
        unsafe { uacpi_table_find_by_signature(c"HPET".as_ptr(), &raw mut table) };
    if status != UACPI_STATUS_OK {
        log::error!("HPET: uacpi_table_find_by_signature failed: {}", unsafe {
            CStr::from_ptr(uacpi_status_to_string(status)).display()
        });
    }

    let hpet = unsafe { table.__bindgen_anon_1.ptr } as *const acpi_hpet;
    let physical_address = unsafe { *hpet }.address.address;

    unsafe {
        uacpi_table_unref(&raw mut table);
    }

    vmm::map_into_kernel(physical_address, Attribute::WRITE);

    unsafe {
        VIRTUAL_ADDRESS = physical_address + boot::get_hhdm_offset();
    }

    log::debug!(
        "HPET: Mapping MMIO {:#018x} -> {:#018x}",
        physical_address,
        unsafe { VIRTUAL_ADDRESS }
    );

    unsafe {
        CLOCK_PERIOD = mmio::read::<u32>(VIRTUAL_ADDRESS + GENERAL_CAPABILITIES_REGISTER + 0x04);
    }

    log::debug!(
        "HPET: Clock period configured with {}ns",
        unsafe { CLOCK_PERIOD } / 1000000
    );

    unsafe {
        mmio::write::<u64>(
            VIRTUAL_ADDRESS + GENERAL_CONFIGURATION_REGISTER,
            mmio::read::<u64>(VIRTUAL_ADDRESS + GENERAL_CONFIGURATION_REGISTER) & !(1u64 << 0),
        );
        mmio::write::<u64>(VIRTUAL_ADDRESS + MAIN_COUNTER_REGISTER, 0);
        mmio::write::<u64>(
            VIRTUAL_ADDRESS + GENERAL_CONFIGURATION_REGISTER,
            mmio::read::<u64>(VIRTUAL_ADDRESS + GENERAL_CONFIGURATION_REGISTER) | 0b1,
        );
    }

    log::debug!("HPET: Reset and enabled timer");

    log::info!("HPET: Initialized");
}

pub fn sleep(ms: u64) {
    assert!(ms > 0);

    unsafe {
        let target_ticks = mmio::read::<u64>(VIRTUAL_ADDRESS + MAIN_COUNTER_REGISTER)
            + (ms * 1000000000000) / CLOCK_PERIOD as u64;

        while mmio::read::<u64>(VIRTUAL_ADDRESS + MAIN_COUNTER_REGISTER) < target_ticks {}
    }
}
