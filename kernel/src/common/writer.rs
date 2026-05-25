//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{
    fmt::{Arguments, Result, Write},
    ptr,
    sync::atomic::{AtomicBool, Ordering},
};

use flanterm_sys::{flanterm_clear, flanterm_context, flanterm_fb_init, flanterm_write};

use crate::{common::boot, drivers::serial};

static mut FLANTERM_CTX: *mut flanterm_context = ptr::null_mut();
static FLANTERM_ENABLED: AtomicBool = AtomicBool::new(false);

struct Writer;

impl Write for Writer {
    fn write_str(&mut self, string: &str) -> Result {
        let flanterm_enabled = FLANTERM_ENABLED.load(Ordering::Relaxed);
        for byte in string.bytes() {
            if byte == b'\n' {
                unsafe {
                    if flanterm_enabled {
                        flanterm_write(FLANTERM_CTX, b"\r\n".as_ptr() as *const i8, 2);
                    }
                    serial::write('\r');
                    serial::write('\n');
                }
            } else {
                unsafe {
                    if flanterm_enabled {
                        flanterm_write(FLANTERM_CTX, &byte as *const u8 as *const i8, 1);
                    }
                    serial::write(byte as char);
                }
            }
        }

        Ok(())
    }
}

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::common::writer::_print(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! println {
    () => (print!("\n"));
    ($($arg:tt)*) => (print!("{}\n", format_args!($($arg)*)));
}

#[doc(hidden)]
pub fn _print(args: Arguments) {
    let mut writer = Writer;
    writer.write_fmt(args).unwrap();
}

pub fn initialize() {
    serial::initialize();

    let framebuffer = boot::get_framebuffers().first().unwrap();

    unsafe {
        FLANTERM_CTX = flanterm_fb_init(
            None,
            None,
            framebuffer.address() as *mut u32,
            framebuffer.width as _,
            framebuffer.height as _,
            framebuffer.pitch as _,
            framebuffer.red_mask_size as _,
            framebuffer.red_mask_shift as _,
            framebuffer.green_mask_size as _,
            framebuffer.green_mask_shift as _,
            framebuffer.blue_mask_size as _,
            framebuffer.blue_mask_shift as _,
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            ptr::null_mut(),
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        );
    }

    FLANTERM_ENABLED.store(true, Ordering::Relaxed);
}

pub fn disable_flanterm() {
    FLANTERM_ENABLED.store(false, Ordering::Relaxed);
    unsafe {
        flanterm_clear(FLANTERM_CTX, true);
    }
}
