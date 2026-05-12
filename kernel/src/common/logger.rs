//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{arch::asm, sync::atomic::Ordering};

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
pub static TSC_BOOT: Once<u64> = Once::new();

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
        const GREEN: &str = "\x1b[38;2;0;128;0m";
        const YELLOW: &str = "\x1b[38;2;255;215;0m";
        const RED: &str = "\x1b[38;2;255;0;0m";
        const BLUE: &str = "\x1b[38;2;0;0;255m";
        const MAGENTA: &str = "\x1b[38;2;170;68;255m";

        const GRAY: &str = "\x1b[38;2;60;60;60m";
        const ITALIC: &str = "\x1b[3m";
        const RESET: &str = "\x1b[0m";

        if !self.enabled(record.metadata()) {
            return;
        }

        let core = Core::current();
        core.enter_critical();

        let _guard = LOCK.lock();

        let timestamp = get_timestamp();
        print!(
            " {}{}.{:03} ",
            GRAY, timestamp.seconds, timestamp.milliseconds
        );

        match record.level() {
            Level::Info => print!("{}info", GREEN),
            Level::Warn => print!("{}warn", YELLOW),
            Level::Error => print!("{}error", RED),
            Level::Debug => print!("{}debug", BLUE),
            Level::Trace => print!("{}trace", MAGENTA),
        }

        print!("{}: ", RESET);

        let path_offset = "kernel/src/".len();
        let file = &record.file().unwrap()[path_offset..];
        let line = record.line().unwrap();
        print!("{}{}<{}:{}> ", GRAY, ITALIC, file, line);

        print!("{}{}[#{}] ", GRAY, ITALIC, core.id);

        let current_thread = core.current_thread.load(Ordering::Relaxed);
        if !current_thread.is_null() {
            unsafe {
                print!(
                    "{}{}({}:{}) ",
                    GRAY,
                    ITALIC,
                    (&(*current_thread)).process.id.0,
                    (*current_thread).id.0
                );
            }
        }

        println!("{}{}", RESET, record.args());

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
