//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::ptr;

use limine::memmap;

use crate::{
    common::{boot, once::Once},
    memory::{PAGE_SIZE, address::PhysicalAddress, bitmap::Bitmap},
    sync::spinlock::SpinLock,
};

static HIGHEST_ADDRESS: Once<PhysicalAddress> = Once::new();
static BITMAP: SpinLock<Bitmap> = SpinLock::new(Bitmap::new(ptr::null_mut(), 0));

pub fn initialize() {
    let memory_map = boot::get_memory_map();

    log::debug!("PMM: Scanning {} memory map entries", memory_map.len());

    let Some(highest_address) = memory_map
        .iter()
        .filter(|entry| entry.type_ == memmap::MEMMAP_USABLE)
        .map(|entry| PhysicalAddress::new(entry.base + entry.length))
        .max()
    else {
        panic!("no usable regions in memory map");
    };

    unsafe {
        HIGHEST_ADDRESS.initialize(highest_address);
    }

    let total_pages = HIGHEST_ADDRESS.get().as_u64().div_ceil(PAGE_SIZE);

    let bitmap_bytes = total_pages.div_ceil(8);
    let bitmap_pages = bitmap_bytes.div_ceil(PAGE_SIZE);

    let Some(bitmap_base) = memory_map
        .iter()
        .find(|entry| entry.type_ == memmap::MEMMAP_USABLE && entry.length >= bitmap_pages * 8)
        .map(|entry| PhysicalAddress::new(entry.base))
    else {
        panic!("no usable region to hold the bitmap");
    };

    let mut bitmap = BITMAP.lock();

    *bitmap = Bitmap::new(
        bitmap_base.to_higher_half().as_mut_ptr(),
        bitmap_bytes as usize,
    );
    bitmap.fill(0xff);

    log::debug!(
        "PMM: Placed bitmap at {:#018x} with a size of {} bytes ({} pages)",
        bitmap.data() as u64,
        bitmap.len(),
        bitmap_pages,
    );

    let mut free_pages = 0;

    for entry in memory_map {
        if entry.type_ != memmap::MEMMAP_USABLE {
            continue;
        }

        assert_eq!(entry.base % PAGE_SIZE, 0);
        assert_eq!(entry.length % PAGE_SIZE, 0);

        let first = entry.base / PAGE_SIZE;
        let count = entry.length / PAGE_SIZE;

        for page in first..first + count {
            bitmap.set(page as usize, false);
        }

        free_pages += count;
    }

    let first_page = bitmap_base.as_u64() / PAGE_SIZE;
    for page in first_page..first_page + bitmap_pages {
        bitmap.set(page as usize, true);
    }
    free_pages -= bitmap_pages;

    log::debug!(
        "PMM: Detected {} free pages ({} MiB) out of {}",
        free_pages,
        (free_pages * PAGE_SIZE) / (1024 * 1024),
        total_pages,
    );

    log::info!("PMM: Initialized");
}

pub fn allocate(pages: u64, clear: bool) -> Option<PhysicalAddress> {
    assert!(pages > 0);

    let mut current_pages = 0;
    let mut bitmap = BITMAP.lock();

    let total = HIGHEST_ADDRESS.get().as_u64() / PAGE_SIZE;
    for i in 0..total {
        if bitmap.is_set(i as usize) {
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
            bitmap.set(j as usize, true);
        }

        let address = PhysicalAddress::new(page * PAGE_SIZE);

        if clear {
            let ptr = address.to_higher_half().as_mut_ptr::<u8>();
            unsafe {
                ptr.write_bytes(0, (pages * PAGE_SIZE) as usize);
            }
        }

        return Some(address);
    }

    // TODO: Replace with error
    None
}

pub fn free(address: PhysicalAddress, pages: u64) {
    let page = address.as_u64() / PAGE_SIZE;

    let mut bitmap = BITMAP.lock();
    for i in 0..pages {
        bitmap.set((page + i) as usize, false);
    }
}
