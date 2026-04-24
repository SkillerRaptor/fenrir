//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![no_std]
#![no_main]

use core::{arch::asm, panic::PanicInfo};

use limine::{BaseRevision, RequestsEndMarker, RequestsStartMarker, request::FramebufferRequest};

#[used]
#[unsafe(link_section = ".limine_requests")]
static BASE_REVISION: BaseRevision = BaseRevision::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
static FRAMEBUFFER_REQUEST: FramebufferRequest = FramebufferRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests_start")]
static _START_MARKER: RequestsStartMarker = RequestsStartMarker::new();

#[used]
#[unsafe(link_section = ".limine_requests_end")]
static _END_MARKER: RequestsEndMarker = RequestsEndMarker::new();

#[unsafe(no_mangle)]
unsafe extern "C" fn kmain() -> ! {
    assert!(BASE_REVISION.is_supported());

    let framebuffer = FRAMEBUFFER_REQUEST
        .response()
        .unwrap()
        .framebuffers()
        .first()
        .unwrap();

    let ptr = framebuffer.address() as *mut u32;
    for y in 0..framebuffer.height {
        for x in 0..framebuffer.width {
            let offset = (y * (framebuffer.pitch / 4) + x) as usize;
            let n_x = (x * 255 / framebuffer.width) as u32;
            let n_y = (y * 255 / framebuffer.height) as u32;
            unsafe {
                ptr.add(offset).write((n_y << 8) | n_x);
            }
        }
    }

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
