//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::arch::asm;

use log::{Level, LevelFilter, Log, Metadata, Record};

use crate::{
    common::{boot, writer},
    print,
    println,
};

static LOGGER: Logger = Logger;
static mut TSC_FREQUENCY: u64 = 0;
static mut TSC_BOOT: u64 = 0;

struct Timestamp {
    seconds: u64,
    milliseconds: u64,
}

fn get_tsc() -> u64 {
    let mut low = 0u32;
    let mut high = 0u32;
    unsafe {
        asm!(
            "rdtsc",
            out("eax") low,
            out("edx") high,
            options(nomem, nostack));
    }

    return ((high as u64) << 32) | (low as u64);
}

fn get_timestamp() -> Timestamp {
    let elapsed = get_tsc() - unsafe { TSC_BOOT };
    let seconds = elapsed / unsafe { TSC_FREQUENCY };
    let remainder = elapsed % unsafe { TSC_FREQUENCY };
    let milliseconds = (remainder * 1000) / unsafe { TSC_FREQUENCY };

    Timestamp {
        seconds,
        milliseconds,
    }
}

struct Logger;

impl Log for Logger {
    fn enabled(&self, _: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        if !self.enabled(record.metadata()) {
            return;
        }

        let timestamp = get_timestamp();
        print!(
            " \x1b[38;2;30;30;30m{}.{:03} ",
            timestamp.seconds, timestamp.milliseconds
        );

        match record.level() {
            Level::Info => print!("\x1b[38;2;0;128;0minfo"),
            Level::Warn => print!("\x1b[38;2;255;215;0mwarn"),
            Level::Error => print!("\x1b[38;2;255;0;0merror"),
            Level::Debug => print!("\x1b[38;2;0;0;255mdebug"),
            Level::Trace => print!("\x1b[38;2;170;68;255mtrace"),
        }

        print!("\x1b[0m: ");
        println!("\x1b[0m{}", record.args());
    }

    fn flush(&self) {}
}

pub fn initialize() {
    unsafe {
        TSC_BOOT = get_tsc();
        TSC_FREQUENCY = boot::get_tsc_frequency();
    }

    writer::initialize();

    let log_level = if cfg!(debug_assertions) {
        LevelFilter::Trace
    } else {
        LevelFilter::Info
    };

    log::set_logger(&LOGGER)
        .map(|()| log::set_max_level(log_level))
        .unwrap();
}
