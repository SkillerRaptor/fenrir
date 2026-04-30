//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::{boxed::Box, vec::Vec};
use core::{arch::asm, mem::offset_of, sync::atomic::AtomicPtr};

use crate::common::{boot, once::Once};

static mut BSP_CORE: Core = Core {
    this: AtomicPtr::null(),
    id: 0,
    lapic_id: 0,
    critical_depth: 0,
    were_interrupts_enabled: false,
};

static CORES: Once<Box<[Core]>> = Once::new();

#[repr(C)]
pub struct Core {
    pub this: AtomicPtr<Core>,

    pub id: u32,
    pub lapic_id: u32,

    pub critical_depth: u32,
    pub were_interrupts_enabled: bool,
}

impl Core {
    pub fn current() -> &'static mut Self {
        let core: *mut Self;
        unsafe {
            asm!(
                "mov {core}, gs:[{offset}]",
                core = out(reg) core,
                offset = const offset_of!(Self, this),
                options(nostack, preserves_flags)
            );
        }

        unsafe { &mut *core }
    }

    pub fn by_id(id: usize) -> &'static mut Self {
        if id == 0 {
            unsafe { &mut *&raw mut BSP_CORE }
        } else {
            &mut CORES.get_mut()[id - 1]
        }
    }

    pub fn enter_critical(&mut self) {
        let were_interrupts_enabled = are_interrupts_enabled();
        disable_interrupts();

        self.critical_depth += 1;
        self.were_interrupts_enabled = were_interrupts_enabled;
    }

    pub fn leave_critical(&mut self) {
        assert!(self.critical_depth > 0);

        self.critical_depth -= 1;

        if self.critical_depth == 0 && self.were_interrupts_enabled {
            self.were_interrupts_enabled = false;
            enable_interrupts();
        }
    }
}

pub fn initialize_bsp() {
    unsafe {
        BSP_CORE = Core {
            this: AtomicPtr::new(&raw mut BSP_CORE),
            id: 0,
            lapic_id: boot::get_bsp_lapic_id(),
            critical_depth: 0,
            were_interrupts_enabled: false,
        };

        set_current_core(&raw mut BSP_CORE);
    }
}

pub fn initialize_cores() {
    let mp_infos = boot::get_mp_infos();

    let mut cores = Vec::with_capacity(mp_infos.len());
    for (i, core) in mp_infos.iter().enumerate() {
        if i == 0 {
            continue;
        }

        cores.push(Core {
            this: AtomicPtr::default(),
            id: i as u32,
            lapic_id: core.lapic_id,
            critical_depth: 0,
            were_interrupts_enabled: false,
        });
    }

    let mut cores = cores.into_boxed_slice();

    for core in &mut cores {
        core.this = AtomicPtr::new(&raw mut *core);
    }

    unsafe {
        CORES.initialize(cores);
    }
}

pub fn set_current_core(core: *mut Core) {
    let address = core as u64;
    set_gs_base(address);
    set_kernel_gs_base(address);
}

pub fn halt() {
    unsafe {
        asm!("hlt", options(nomem, nostack, preserves_flags));
    }
}

pub fn pause() {
    unsafe {
        asm!("pause", options(nomem, nostack, preserves_flags));
    }
}

pub fn enable_interrupts() {
    unsafe {
        asm!("sti", options(nomem, nostack, preserves_flags));
    }
}

pub fn disable_interrupts() {
    unsafe {
        asm!("cli", options(nomem, nostack, preserves_flags));
    }
}

pub fn hcf() -> ! {
    loop {
        disable_interrupts();
        halt();
    }
}

pub fn read_cr0() -> u64 {
    let value;

    unsafe {
        asm!(
            "mov {}, cr0",
            out(reg) value,
            options(nomem, nostack, preserves_flags)
        );
    }

    value
}

pub fn read_cr2() -> u64 {
    let value;

    unsafe {
        asm!(
            "mov {}, cr2",
            out(reg) value,
            options(nomem, nostack, preserves_flags)
        );
    }

    value
}

pub fn read_cr3() -> u64 {
    let value;

    unsafe {
        asm!(
            "mov {}, cr3",
            out(reg) value,
            options(nomem, nostack, preserves_flags)
        );
    }

    value
}

pub fn read_cr4() -> u64 {
    let value;

    unsafe {
        asm!(
            "mov {}, cr4",
            out(reg) value,
            options(nomem, nostack, preserves_flags)
        );
    }

    value
}

pub fn flags() -> u64 {
    let value;

    unsafe {
        asm!(
            "pushf",
            "pop {}",
            out(reg) value,
            options(nomem, preserves_flags)
        );
    }

    value
}

pub fn are_interrupts_enabled() -> bool {
    (flags() & (1 << 9)) != 0
}

pub fn write_msr(msr: u32, value: u64) {
    let high = (value >> 32) & 0xffffffff;
    let low = (value >> 0) & 0xffffffff;

    unsafe {
        asm!(
            "wrmsr",
            in("ecx") msr,
            in("eax") low,
            in("edx") high,
            options(nostack, preserves_flags)
        );
    }
}

pub fn read_msr(msr: u32) -> u64 {
    let mut high = 0;
    let mut low = 0;

    unsafe {
        asm!(
            "rdmsr",
            in("ecx") msr,
            out("eax") low,
            out("edx") high,
            options(nomem, nostack, preserves_flags)
        );
    }

    ((high as u64) << 32) | (low as u64)
}

pub fn set_fs_base(value: u64) {
    write_msr(0xc0000100, value);
}

pub fn set_gs_base(value: u64) {
    write_msr(0xc0000101, value);
}

pub fn set_kernel_gs_base(value: u64) {
    write_msr(0xc0000102, value);
}
