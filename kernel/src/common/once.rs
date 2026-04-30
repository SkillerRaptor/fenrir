//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{cell::UnsafeCell, mem::MaybeUninit};

pub struct Once<T> {
    data: UnsafeCell<MaybeUninit<T>>,
}

impl<T> Once<T> {
    pub const fn new() -> Self {
        Self {
            data: UnsafeCell::new(MaybeUninit::uninit()),
        }
    }

    pub unsafe fn initialize(&self, data: T) {
        unsafe {
            (*self.data.get()).write(data);
        }
    }

    pub fn get(&self) -> &T {
        unsafe { (*self.data.get()).assume_init_ref() }
    }

    pub fn get_mut(&self) -> &mut T {
        unsafe { (*self.data.get()).assume_init_mut() }
    }

    pub unsafe fn get_ptr(&self) -> *const T {
        unsafe { (*self.data.get()).as_ptr() }
    }
}

unsafe impl<T: Sync> Sync for Once<T> {}
