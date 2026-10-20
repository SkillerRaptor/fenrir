//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub struct Bitmap {
    data: *mut u8,
    len: usize,
}

impl Bitmap {
    pub const fn new(data: *mut u8, len: usize) -> Self {
        Self { data, len }
    }

    pub fn set(&mut self, index: usize, value: bool) {
        if index >= self.bit_len() {
            panic!(
                "index out of bounds: the len is {} bits but the index is {}",
                self.bit_len(),
                index
            );
        }

        let byte_index = index / 8;
        let bit_index = index % 8;

        let byte_ptr = unsafe { self.data.add(byte_index) };
        let byte_value = unsafe { byte_ptr.read_volatile() };

        let new_value = if value {
            byte_value | 1 << bit_index
        } else {
            byte_value & !(1 << bit_index)
        };

        unsafe {
            byte_ptr.write_volatile(new_value);
        }
    }

    pub fn is_set(&self, index: usize) -> bool {
        if index >= self.bit_len() {
            panic!(
                "index out of bounds: the len is {} bits but the index is {}",
                self.bit_len(),
                index
            );
        }

        let byte_index = index / 8;
        let bit_index = index % 8;

        let byte_ptr = unsafe { self.data.add(byte_index) };
        let byte_value = unsafe { byte_ptr.read_volatile() };

        let bit_value = byte_value & (1 << bit_index);
        bit_value != 0
    }

    pub fn fill(&mut self, byte: u8) {
        unsafe {
            self.data.write_bytes(byte, self.len as usize);
        }
    }

    pub fn data(&self) -> *mut u8 {
        self.data
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn bit_len(&self) -> usize {
        self.len * 8
    }
}

unsafe impl Send for Bitmap {}
