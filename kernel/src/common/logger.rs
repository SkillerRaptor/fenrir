//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use crate::{common::writer, print, println};

use log::{Level, LevelFilter, Log, Metadata, Record};

static LOGGER: Logger = Logger;

struct Logger;

impl Log for Logger {
    fn enabled(&self, _: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        if !self.enabled(record.metadata()) {
            return;
        }

        match record.level() {
            Level::Info => print!("\x1b[38;2;0;128;0minfo"),
            Level::Warn => print!("\x1b[38;2;255;215;0mwarn"),
            Level::Error => print!("\x1b[38;2;255;0;0merror"),
            Level::Debug => print!("\x1b[38;2;0;0;255mdebug"),
            Level::Trace => print!("\x1b[38;2;170;68;255mtrace"),
        }

        println!("\x1b[0m: {}", record.args());
    }

    fn flush(&self) {}
}

pub fn initialize() {
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
