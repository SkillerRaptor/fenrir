//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{
    alloc::{GlobalAlloc, Layout},
    ptr,
};

use crate::{
    common::{boot, math},
    memory::{PAGE_SIZE, address::PhysicalAddress, pmm},
};

#[repr(C)]
struct AllocationHeader {
    page_count: u64,
    size: usize,
}

struct KernelAllocator;

unsafe impl GlobalAlloc for KernelAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let size = layout.size();
        assert!(size > 0);

        let page_count = math::div_round_up(size as u64, PAGE_SIZE);

        let Some(ptr) = pmm::allocate(page_count + 1, false) else {
            return ptr::null_mut();
        };

        let ptr = ptr.to_higher_half().as_mut_ptr::<u8>();
        let header = ptr as *mut AllocationHeader;
        unsafe {
            (*header).page_count = page_count;
            (*header).size = size;
        }

        unsafe { ptr.add(PAGE_SIZE as usize) }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        if ptr == ptr::null_mut() {
            return;
        }

        let header_address = unsafe { ptr.sub(PAGE_SIZE as usize) };
        let header = header_address as *const AllocationHeader;

        let page_count = unsafe { (*header).page_count + 1 };
        let physical = PhysicalAddress::new(header_address as u64 - boot::get_hhdm_offset());

        pmm::free(physical, page_count);
    }
}

#[global_allocator]
static ALLOCATOR: KernelAllocator = KernelAllocator;
