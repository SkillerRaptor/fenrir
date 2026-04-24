//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![no_std]
#![no_main]

mod common;

use core::{arch::asm, panic::PanicInfo};

use limine::{BaseRevision, RequestsEndMarker, RequestsStartMarker};

use crate::common::{logger, writer};

#[used]
#[unsafe(link_section = ".limine_requests")]
static BASE_REVISION: BaseRevision = BaseRevision::new();

#[used]
#[unsafe(link_section = ".limine_requests_start")]
static _START_MARKER: RequestsStartMarker = RequestsStartMarker::new();

#[used]
#[unsafe(link_section = ".limine_requests_end")]
static _END_MARKER: RequestsEndMarker = RequestsEndMarker::new();

#[unsafe(no_mangle)]
unsafe extern "C" fn kmain() -> ! {
    assert!(BASE_REVISION.is_supported());

    writer::initialize();
    logger::initialize();

    log::info!("Hello, World!");

    loop {
        unsafe {
            asm!("hlt");
        }
    }
}

#[panic_handler]
fn rust_panic(_info: &PanicInfo) -> ! {
    loop {
        unsafe {
            asm!("cli");
            asm!("hlt");
        }
    }
}
