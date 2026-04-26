//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{
    ptr,
    sync::atomic::{AtomicU64, Ordering},
};

use limine::memmap::{
    MEMMAP_ACPI_NVS,
    MEMMAP_ACPI_RECLAIMABLE,
    MEMMAP_BAD_MEMORY,
    MEMMAP_BOOTLOADER_RECLAIMABLE,
    MEMMAP_EXECUTABLE_AND_MODULES,
    MEMMAP_FRAMEBUFFER,
    MEMMAP_MAPPED_RESERVED,
    MEMMAP_RESERVED,
    MEMMAP_USABLE,
};

use crate::{
    common::{boot, math},
    memory::{PAGE_SIZE, bitmap::Bitmap},
    sync::spinlock::SpinLock,
};

static HIGHEST_PAGE: AtomicU64 = AtomicU64::new(0);
static BITMAP: SpinLock<Bitmap> = SpinLock::new(Bitmap::default());

pub fn initialize() {
    let memory_map = boot::get_memory_map();

    log::debug!("PMM: Scanning {} memory map entries", memory_map.len());

    for entry in memory_map {
        let ty = match entry.type_ {
            MEMMAP_USABLE => "Usable",
            MEMMAP_RESERVED => "Reserved",
            MEMMAP_ACPI_RECLAIMABLE => "ACPI Reclaimable",
            MEMMAP_ACPI_NVS => "ACPI NVS",
            MEMMAP_BAD_MEMORY => "Bad Memory",
            MEMMAP_BOOTLOADER_RECLAIMABLE => "Bootloader Reclaimable",
            MEMMAP_EXECUTABLE_AND_MODULES => "Executable and Modules",
            MEMMAP_FRAMEBUFFER => "Framebuffer",
            MEMMAP_MAPPED_RESERVED => "Mapped Reserved",
            _ => unreachable!(),
        };

        log::debug!(
            "PMM:   [{:#018x}-{:#018x}] {}",
            entry.base,
            entry.base + entry.length,
            ty
        );

        if entry.type_ != MEMMAP_USABLE {
            continue;
        }

        let end_of_page = entry.base + entry.length;

        if end_of_page > HIGHEST_PAGE.load(Ordering::SeqCst) {
            HIGHEST_PAGE.store(end_of_page, Ordering::SeqCst);
        }
    }

    let size = math::div_round_up(HIGHEST_PAGE.load(Ordering::SeqCst), PAGE_SIZE);

    for entry in memory_map {
        if entry.type_ != MEMMAP_USABLE {
            continue;
        }

        if entry.length >= size {
            let data = (entry.base + boot::get_hhdm_offset()) as *mut u8;

            let mut bitmap = BITMAP.lock();
            *bitmap = Bitmap::new(data, size);
            bitmap.fill(0xff);

            break;
        }
    }

    {
        let bitmap = BITMAP.lock();
        log::debug!(
            "PMM: Placed bitmap at {:#018x} with a size of {} bytes ({} KiB)",
            bitmap.data() as u64,
            bitmap.size() / 8,
            bitmap.size() / 8 / 1024,
        );
    }

    let mut free_pages = 0;

    for entry in memory_map {
        if entry.type_ != MEMMAP_USABLE {
            continue;
        }

        let mut bitmap = BITMAP.lock();
        if (entry.base + boot::get_hhdm_offset()) as *mut u8 == bitmap.data() {
            let remaining_bytes = entry.length - bitmap.size();
            for offset in (0..remaining_bytes).step_by(PAGE_SIZE as usize) {
                let address = (entry.base + bitmap.size()) + offset;
                let page_index = address / PAGE_SIZE;
                bitmap.set(page_index, false);
            }

            free_pages += remaining_bytes / PAGE_SIZE;

            continue;
        }

        for offset in (0..entry.length).step_by(PAGE_SIZE as usize) {
            let address = entry.base + offset;
            let page_index = address / PAGE_SIZE;
            bitmap.set(page_index, false);
        }

        free_pages += entry.length / PAGE_SIZE;
    }

    log::debug!(
        "PMM: Detected {} free pages ({} KiB, {} MiB)",
        free_pages,
        (free_pages * PAGE_SIZE) / 1024,
        (free_pages * PAGE_SIZE) / 1024 / 1024,
    );

    log::info!("PMM: Initialized");
}

pub fn allocate(pages: u64, clear: bool) -> *mut u8 {
    assert!(pages > 0);

    let mut current_pages = 0;
    let mut bitmap = BITMAP.lock();

    let total = HIGHEST_PAGE.load(Ordering::Relaxed) / PAGE_SIZE;
    for i in 0..total {
        if bitmap.get(i) {
            current_pages = 0;
            continue;
        }

        current_pages += 1;
        if current_pages != pages {
            continue;
        }

        let index = i + 1;
        let page = index - pages;
        for j in page..index {
            bitmap.set(j, true);
        }

        let ptr = (page * PAGE_SIZE) as *mut u8;

        if clear {
            let address = ((ptr as u64) + boot::get_hhdm_offset()) as *mut u64;
            for current_page in 0..(pages * (PAGE_SIZE / size_of::<u64>() as u64)) {
                unsafe { address.add(current_page as usize).write(0) };
            }
        }

        return ptr;
    }

    ptr::null_mut()
}

pub fn free(ptr: *mut u8, pages: u64) {
    let address = ptr as u64;
    let page = address / PAGE_SIZE;

    let mut bitmap = BITMAP.lock();
    for i in 0..pages {
        bitmap.set(page + i as u64, false);
    }
}
