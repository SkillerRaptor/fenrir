//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use core::{
    fmt::{self, Debug},
    ops::{Add, AddAssign, Sub, SubAssign},
};

use crate::common::{boot, math};

macro_rules! implement_address_type {
    ($name:ident) => {
        #[repr(transparent)]
        #[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
        pub struct $name(u64);

        impl $name {
            pub const fn new(address: u64) -> Self {
                Self(address)
            }

            pub fn align_up(self, alignment: u64) -> Self {
                Self::new(math::align_up(self.0, alignment))
            }

            pub fn align_down(self, alignment: u64) -> Self {
                Self::new(math::align_down(self.0, alignment))
            }

            pub fn is_aligned(self, alignment: u64) -> bool {
                (self.0 & alignment) == 0
            }

            pub fn as_u64(self) -> u64 {
                self.0
            }
        }

        impl Add<u64> for $name {
            type Output = Self;

            fn add(self, rhs: u64) -> Self::Output {
                // TODO: Implement proper error message
                Self::new(self.0.checked_add(rhs).unwrap())
            }
        }

        impl Sub<u64> for $name {
            type Output = Self;

            fn sub(self, rhs: u64) -> Self::Output {
                // TODO: Implement proper error message
                Self::new(self.0.checked_sub(rhs).unwrap())
            }
        }

        impl AddAssign<u64> for $name {
            fn add_assign(&mut self, rhs: u64) {
                *self = *self + rhs;
            }
        }

        impl SubAssign<u64> for $name {
            fn sub_assign(&mut self, rhs: u64) {
                *self = *self - rhs;
            }
        }

        impl Debug for $name {
            fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                write!(f, concat!(stringify!($name), "({:#018x})"), self.0)
            }
        }
    };
}

implement_address_type!(PhysicalAddress);

impl PhysicalAddress {
    pub fn to_higher_half(self) -> VirtualAddress {
        // TODO: Implement proper error message
        VirtualAddress(self.0.checked_add(boot::get_hhdm_offset()).unwrap())
    }
}

implement_address_type!(VirtualAddress);

impl VirtualAddress {
    pub fn from_ptr<T: ?Sized>(ptr: *const T) -> Self {
        Self::new(ptr as *const () as u64)
    }

    pub fn as_ptr<T>(self) -> *const T {
        self.as_u64() as *const T
    }

    pub fn as_mut_ptr<T>(self) -> *mut T {
        self.as_u64() as *mut T
    }
}
