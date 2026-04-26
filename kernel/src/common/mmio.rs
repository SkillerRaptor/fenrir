//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

pub trait MmioType {}

impl MmioType for u8 {}
impl MmioType for u16 {}
impl MmioType for u32 {}
impl MmioType for u64 {}

pub unsafe fn read<T>(address: u64) -> T
where
    T: MmioType,
{
    unsafe { (address as *const T).read_volatile() }
}

pub unsafe fn write<T>(address: u64, value: T)
where
    T: MmioType,
{
    unsafe { (address as *mut T).write_volatile(value) };
}
