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
    common::{boot, mmio, once::Once},
    memory::vmm::{self, Attribute},
};

const GENERAL_CAPABILITIES_REGISTER: u64 = 0x000;
const GENERAL_CONFIGURATION_REGISTER: u64 = 0x010;
const MAIN_COUNTER_REGISTER: u64 = 0x0f0;

static VIRTUAL_ADDRESS: Once<u64> = Once::new();
static CLOCK_PERIOD: Once<u32> = Once::new();

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
        VIRTUAL_ADDRESS.initialize(physical_address + boot::get_hhdm_offset());
    }

    log::debug!(
        "HPET: Mapping MMIO {:#018x} -> {:#018x}",
        physical_address,
        VIRTUAL_ADDRESS.get()
    );

    unsafe {
        CLOCK_PERIOD.initialize((read(GENERAL_CAPABILITIES_REGISTER) >> 32 & 0xffffffff) as u32);
    }

    log::debug!(
        "HPET: Clock period configured with {}ns",
        CLOCK_PERIOD.get() / 1000000
    );

    write(
        GENERAL_CONFIGURATION_REGISTER,
        read(GENERAL_CONFIGURATION_REGISTER) & !(1u64 << 0),
    );
    write(MAIN_COUNTER_REGISTER, 0);
    write(
        GENERAL_CONFIGURATION_REGISTER,
        read(GENERAL_CONFIGURATION_REGISTER) | 0b1,
    );

    log::debug!("HPET: Reset and enabled timer");

    log::info!("HPET: Initialized");
}

pub fn sleep(ms: u64) {
    assert!(ms > 0);

    let target_ticks =
        read(MAIN_COUNTER_REGISTER) + (ms * 1000000000000) / *CLOCK_PERIOD.get() as u64;

    while read(MAIN_COUNTER_REGISTER) < target_ticks {}
}

fn write(offset: u64, value: u64) {
    unsafe {
        mmio::write(VIRTUAL_ADDRESS.get() + offset, value);
    }
}

fn read(offset: u64) -> u64 {
    unsafe { mmio::read::<u64>(VIRTUAL_ADDRESS.get() + offset) }
}
