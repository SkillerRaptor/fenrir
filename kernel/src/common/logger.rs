//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::arch::asm;

use log::{Level, LevelFilter, Log, Metadata, Record};

use crate::{
    arch::x86_64::cpu::Core,
    common::{boot, once::Once, writer},
    print,
    println,
    sync::spinlock::SpinLock,
};

static LOGGER: Logger = Logger;
static LOCK: SpinLock<()> = SpinLock::new(());

static TSC_FREQUENCY: Once<u64> = Once::new();
static TSC_BOOT: Once<u64> = Once::new();

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
    let elapsed = get_tsc() - TSC_BOOT.get();
    let seconds = elapsed / TSC_FREQUENCY.get();
    let remainder = elapsed % TSC_FREQUENCY.get();
    let milliseconds = (remainder * 1000) / TSC_FREQUENCY.get();

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

        let core = Core::current();
        core.enter_critical();

        let _guard = LOCK.lock();

        let timestamp = get_timestamp();
        print!(
            " \x1b[38;2;80;80;80m{}.{:03} ",
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

        core.leave_critical();
    }

    fn flush(&self) {}
}

pub fn initialize() {
    unsafe {
        TSC_BOOT.initialize(get_tsc());
        TSC_FREQUENCY.initialize(boot::get_tsc_frequency());
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
