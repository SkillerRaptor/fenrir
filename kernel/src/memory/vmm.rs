//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{arch::asm, mem};

use bitflags::bitflags;
use limine::memmap::{
    MEMMAP_ACPI_RECLAIMABLE,
    MEMMAP_BOOTLOADER_RECLAIMABLE,
    MEMMAP_EXECUTABLE_AND_MODULES,
    MEMMAP_FRAMEBUFFER,
    MEMMAP_USABLE,
};

use crate::{
    common::{boot, math},
    memory::{PAGE_SIZE, pmm},
};

bitflags! {
    pub struct Attribute: u16 {
        const NULL = 0;
        const PRESENT = 1 << 0;
        const WRITE = 1 << 1;
        const USER = 1 << 2;
    }
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct PageMap(u64);

const ADDRESS_MASK: u64 = ((1u64 << 36) - 1) << 12;

static mut KERNEL_PAGE_MAP: PageMap = PageMap(0);

unsafe extern "C" {
    static __kernel_start: u8;
    static __kernel_end: u8;
}

pub fn initialize() {
    unsafe {
        KERNEL_PAGE_MAP = create_page_map();
    }

    let memory_map = boot::get_memory_map();

    let mut mapped_entry_count = 0;
    let mut mapped_bytes = 0;
    for entry in memory_map {
        if !matches!(
            entry.type_,
            MEMMAP_USABLE
                | MEMMAP_ACPI_RECLAIMABLE
                | MEMMAP_BOOTLOADER_RECLAIMABLE
                | MEMMAP_EXECUTABLE_AND_MODULES
                | MEMMAP_FRAMEBUFFER
        ) {
            continue;
        }

        let entry_start = math::align_down(entry.base, PAGE_SIZE);
        let entry_end = math::align_up(entry.base + entry.length, PAGE_SIZE);
        let entry_pages = (entry_end - entry_start) / PAGE_SIZE;

        log::debug!(
            "VMM:   [{:#018x}-{:#018x}] -> [{:#018x}-{:#018x}] ({} pages)",
            entry_start,
            entry_end,
            entry_start + boot::get_hhdm_offset(),
            entry_end + boot::get_hhdm_offset(),
            entry_pages
        );

        for page in (entry_start..entry_end).step_by(PAGE_SIZE as usize) {
            map(
                unsafe { KERNEL_PAGE_MAP },
                page,
                page + boot::get_hhdm_offset(),
                Attribute::WRITE,
            );
        }

        mapped_entry_count += 1;
        mapped_bytes += entry_end - entry_start;
    }

    log::debug!(
        "VMM: Mapped {} entries ({} KiB, {} MiB)",
        mapped_entry_count,
        mapped_bytes / 1024,
        mapped_bytes / 1024 / 1024,
    );

    let kernel_virtual_start =
        math::align_down(unsafe { &__kernel_start as *const u8 as u64 }, PAGE_SIZE);
    let kernel_virtual_end =
        math::align_up(unsafe { &__kernel_end as *const u8 as u64 }, PAGE_SIZE);
    let physical_base = boot::get_executable_physical_base();
    let virtual_base = boot::get_executable_virtual_base();
    let kernel_pages = (kernel_virtual_end - kernel_virtual_start) / PAGE_SIZE;

    log::debug!("VMM: Mapping kernel image...");
    log::debug!(
        "VMM:   [{:#018x}-{:#018x}] -> [{:#018x}-{:#018x}] ({} pages)",
        kernel_virtual_start - virtual_base + physical_base,
        kernel_virtual_end - virtual_base + physical_base,
        kernel_virtual_start,
        kernel_virtual_end,
        kernel_pages
    );

    for page in (kernel_virtual_start..kernel_virtual_end).step_by(PAGE_SIZE as usize) {
        map(
            unsafe { KERNEL_PAGE_MAP },
            page - virtual_base + physical_base,
            page,
            Attribute::WRITE,
        );
    }

    log::debug!(
        "VMM: Mapped kernel ({} KiB)",
        (kernel_virtual_end - kernel_virtual_start) / 1024,
    );

    log::debug!("VMM: Switching to kernel page map...");
    switch_to_page_map(unsafe { KERNEL_PAGE_MAP });

    log::info!("VMM: Initialized");
}

pub fn create_page_map() -> PageMap {
    let page_map = PageMap(pmm::allocate(1, true) as u64);

    // NOTE: This copies the higher half of the kernel page map
    if unsafe { KERNEL_PAGE_MAP.0 } != 0 {
        let hhdm_offset = boot::get_hhdm_offset();
        for i in 256..512 {
            let offset = i * mem::size_of::<u64>() as u64;
            let kernel_entry_address =
                (unsafe { KERNEL_PAGE_MAP.0 } + offset + hhdm_offset) as *const u64;

            let kernel_entry = unsafe { kernel_entry_address.read() };
            let attributes = Attribute::from_bits_truncate((kernel_entry & 0x0fff) as u16);
            if attributes.contains(Attribute::PRESENT) {
                let new_entry_address = (page_map.0 + offset + hhdm_offset) as *mut u64;
                unsafe {
                    new_entry_address.write(kernel_entry);
                }
            }
        }
    }

    page_map
}

pub fn switch_to_page_map(page_map: PageMap) {
    unsafe {
        asm!(
            "mov cr3, {}",
            in(reg) page_map.0,
            options(nostack)
        );
    }
}

fn get_next_level(pml: u64, entry: u16) -> u64 {
    let pml_address = (pml + boot::get_hhdm_offset()) as *mut u64;
    let pml_entry = unsafe { pml_address.add(entry as usize) };

    let attributes = Attribute::from_bits_truncate((unsafe { pml_entry.read() } & 0x0fff) as u16);
    if !attributes.contains(Attribute::PRESENT) {
        let new_level = pmm::allocate(1, true) as u64;
        unsafe {
            pml_entry.write(
                new_level | (Attribute::USER | Attribute::WRITE | Attribute::PRESENT).bits() as u64,
            );
        }
    }

    (unsafe { pml_entry.read() } & ADDRESS_MASK)
}

fn get_pte(page_map: PageMap, virtual_addr: u64) -> *mut u64 {
    let pml4_entry = ((virtual_addr >> 39) & 0x1ff) as u16;
    let pml4 = page_map.0;

    let pdpt_entry = ((virtual_addr >> 30) & 0x1ff) as u16;
    let pdpt = get_next_level(pml4, pml4_entry);

    let pd_entry = ((virtual_addr >> 21) & 0x1ff) as u16;
    let pd = get_next_level(pdpt, pdpt_entry);

    let pt_entry = ((virtual_addr >> 12) & 0x1ff) as u16;
    let pt = get_next_level(pd, pd_entry);

    let entry_address = (pt + boot::get_hhdm_offset()) as *mut u64;
    let entry = unsafe { entry_address.add(pt_entry as usize) };

    entry
}

pub fn map(page_map: PageMap, physical_addr: u64, virtual_addr: u64, attributes: Attribute) {
    let aligned_physical_addr = math::align_down(physical_addr, PAGE_SIZE);
    let aligned_virtual_addr = math::align_down(virtual_addr, PAGE_SIZE);

    let entry = get_pte(page_map, aligned_virtual_addr);
    unsafe {
        entry.write(
            (aligned_physical_addr & ADDRESS_MASK)
                | (attributes | Attribute::PRESENT).bits() as u64,
        );
    }
}

pub fn map_into_kernel(physical_address: u64, attributes: Attribute) {
    map(
        unsafe { KERNEL_PAGE_MAP },
        physical_address,
        physical_address + boot::get_hhdm_offset(),
        attributes,
    );
}

pub fn get_kernel_page_map() -> PageMap {
    unsafe { KERNEL_PAGE_MAP }
}

pub fn virtual_to_physical(page_map: PageMap, virtual_address: u64) -> u64 {
    let entry = get_pte(page_map, virtual_address);
    unsafe { (*entry & ADDRESS_MASK) + (virtual_address & 0xfff) }
}
