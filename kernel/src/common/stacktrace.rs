//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::{string::String, vec::Vec};

use crate::{common::boot, sync::spinlock::SpinLock};

struct Symbol {
    name: String,
    address: u64,
}

static SYMBOLS: SpinLock<Vec<Symbol>> = SpinLock::new(Vec::new());

pub fn initialize() {
    let symbol_map_file = boot::get_modules()[0];

    log::debug!(
        "Stacktrace: Kernel symbol map found ({} KiB)",
        symbol_map_file.data().len() / 1024
    );

    let symbol_text = str::from_utf8(symbol_map_file.data()).unwrap();

    let mut symbols = SYMBOLS.lock();
    for line in symbol_text.lines() {
        if line.len() < 19 {
            continue;
        }

        let (address_string, rest) = line.split_at(16);

        let mut chars = rest.chars();
        if chars.next() != Some(' ') {
            continue;
        }

        chars.next();

        if chars.next() != Some(' ') {
            continue;
        }

        let name = chars.as_str();
        if name.is_empty() {
            continue;
        }

        let Ok(address) = u64::from_str_radix(address_string, 16) else {
            continue;
        };

        symbols.push(Symbol {
            name: String::from(name),
            address,
        });
    }

    log::debug!("Stacktrace: Loaded {} symbols", symbols.len());

    log::info!("Stacktrace: Initialized");
}

#[repr(C)]
struct StackFrame {
    rbp: *const StackFrame,
    rip: u64,
}

fn find_symbol<'a>(rip: u64) -> Option<&'a Symbol> {
    let symbols = SYMBOLS.lock();
    if symbols.is_empty() {
        return None;
    }

    let index = symbols.partition_point(|symbol| symbol.address <= rip);
    if index == 0 {
        return None;
    }

    unsafe {
        let symbol = &symbols[index - 1] as *const Symbol;
        Some(&*symbol)
    }
}

pub fn print(max_frames: u64) {
    let mut stack_frame: *const StackFrame;
    unsafe {
        core::arch::asm!("mov {}, rbp", out(reg) stack_frame, options(nomem, nostack, preserves_flags));
    }

    let mut index = 0;
    while !stack_frame.is_null() && index < max_frames {
        let rip = unsafe { stack_frame.read().rip };
        if rip == 0 {
            break;
        }

        let address = unsafe { stack_frame.read().rbp } as u64;
        if address != 0 && (address & 0x7) != 0 {
            break;
        }

        stack_frame = unsafe { stack_frame.read().rbp } as *const StackFrame;
        index += 1;

        match find_symbol(rip) {
            None => {
                log::error!(
                    "  {:02}. \x1b[38;2;0;0;255m{:#018x} \x1b[0min \x1b[38;2;255;215;0m??",
                    index + 1,
                    rip
                );
            }
            Some(symbol) => {
                let offset = rip - symbol.address;
                if offset == 0 {
                    log::error!(
                        "  {:02}. \x1b[38;2;0;0;255m{:#018x}\x1b[0m in \x1b[38;2;255;215;0m{}\x1b[0m",
                        index,
                        rip,
                        symbol.name
                    );
                } else {
                    log::error!(
                        "  {:02}. \x1b[38;2;0;0;255m{:#018x}\x1b[0m in \x1b[38;2;255;215;0m{}\x1b[0m \x1b[38;2;0;128;0m+{:#x}\x1b[0m",
                        index,
                        rip,
                        symbol.name,
                        offset
                    );
                }
            }
        }
    }
}
