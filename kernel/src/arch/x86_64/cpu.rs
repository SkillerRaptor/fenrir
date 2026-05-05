//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

use alloc::{boxed::Box, collections::VecDeque, sync::Arc, vec::Vec};
use core::{
    arch::asm,
    mem::offset_of,
    ptr,
    sync::atomic::{AtomicBool, AtomicPtr, AtomicU32, Ordering},
};

use crate::{
    common::{boot, once::Once},
    scheduler::thread::Thread,
    sync::spinlock::SpinLock,
};

static BSP_CORE: Once<Core> = Once::new();
static CORES: Once<Box<[Core]>> = Once::new();

#[repr(C)]
pub struct Core {
    pub this: AtomicPtr<Core>,

    pub id: u32,
    pub lapic_id: u32,

    pub critical_depth: AtomicU32,
    pub were_interrupts_enabled: AtomicBool,

    // NOTE: Scheduler stuff
    pub idle_thread: AtomicPtr<Thread>,
    pub current_thread: AtomicPtr<Thread>,
    pub thread_queue: SpinLock<VecDeque<Arc<Thread>>>,
}

impl Core {
    pub fn current() -> &'static Self {
        let core: *const Self;
        unsafe {
            asm!(
                "mov {core}, gs:[{offset}]",
                core = out(reg) core,
                offset = const offset_of!(Self, this),
                options(nostack, preserves_flags)
            );
        }

        unsafe { &*core }
    }

    pub fn by_id(id: usize) -> &'static Self {
        if id == 0 {
            BSP_CORE.get()
        } else {
            &CORES.get()[id - 1]
        }
    }

    pub fn enter_critical(&self) {
        let were_interrupts_enabled = are_interrupts_enabled();
        disable_interrupts();

        if self.critical_depth.fetch_add(1, Ordering::Relaxed) == 0 {
            self.were_interrupts_enabled
                .store(were_interrupts_enabled, Ordering::Relaxed);
        }
    }

    pub fn leave_critical(&self) {
        let critical_depth = self.critical_depth.fetch_sub(1, Ordering::Relaxed);
        assert!(critical_depth > 0);

        if critical_depth == 1 && self.were_interrupts_enabled.load(Ordering::Relaxed) {
            self.were_interrupts_enabled.store(false, Ordering::Relaxed);
            enable_interrupts();
        }
    }
}

pub fn initialize_bsp() {
    unsafe {
        BSP_CORE.initialize(Core {
            this: AtomicPtr::new(ptr::null_mut()),
            id: 0,
            lapic_id: boot::get_bsp_lapic_id(),
            critical_depth: AtomicU32::new(0),
            were_interrupts_enabled: AtomicBool::new(false),
            idle_thread: AtomicPtr::new(ptr::null_mut()),
            current_thread: AtomicPtr::new(ptr::null_mut()),
            thread_queue: SpinLock::new(VecDeque::new()),
        });

        let ptr = BSP_CORE.get() as *const Core as *mut Core;
        BSP_CORE.get().this.store(ptr, Ordering::Relaxed);

        set_current_core(BSP_CORE.get());
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
            this: AtomicPtr::new(ptr::null_mut()),
            id: i as u32,
            lapic_id: core.lapic_id,
            critical_depth: AtomicU32::new(0),
            were_interrupts_enabled: AtomicBool::new(false),
            idle_thread: AtomicPtr::new(ptr::null_mut()),
            current_thread: AtomicPtr::new(ptr::null_mut()),
            thread_queue: SpinLock::new(VecDeque::new()),
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

pub fn set_current_core(core: *const Core) {
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
