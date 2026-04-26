//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::ptr;

pub struct Bitmap {
    data: *mut u8,
    size: u64,
}

unsafe impl Send for Bitmap {}

impl Bitmap {
    pub fn new(data: *mut u8, size: u64) -> Self {
        Self { data, size }
    }

    pub fn fill(&mut self, byte: u8) {
        unsafe {
            self.data.write_bytes(byte, (self.size / 8) as usize);
        }
    }

    pub fn set(&mut self, index: u64, value: bool) {
        assert!(index < self.size);

        let byte = index / 8;
        let bit = index % 8;

        unsafe {
            if value {
                *self.data.add(byte as usize) |= 1 << bit;
            } else {
                *self.data.add(byte as usize) &= !(1 << bit);
            }
        }
    }

    pub fn get(&self, index: u64) -> bool {
        assert!(index < self.size);

        let byte = index / 8;
        let bit = index % 8;

        unsafe { (*self.data.add(byte as usize) & (1 << bit)) != 0 }
    }

    pub fn data(&self) -> *mut u8 {
        self.data
    }

    pub fn size(&self) -> u64 {
        self.size
    }

    pub const fn default() -> Self {
        Self {
            data: ptr::null_mut(),
            size: 0,
        }
    }
}
