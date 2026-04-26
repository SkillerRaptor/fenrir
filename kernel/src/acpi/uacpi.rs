//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

#![allow(unused)]

use alloc::boxed::Box;
use core::{
    alloc::Layout,
    ffi::{CStr, c_void},
    mem,
    ptr,
};

use uacpi_sys::{
    UACPI_LOG_ERROR,
    UACPI_LOG_INFO,
    UACPI_LOG_WARN,
    UACPI_STATUS_OK,
    uacpi_bool,
    uacpi_char,
    uacpi_cpu_flags,
    uacpi_firmware_request,
    uacpi_handle,
    uacpi_interrupt_handler,
    uacpi_interrupt_state,
    uacpi_io_addr,
    uacpi_log_level,
    uacpi_pci_address,
    uacpi_phys_addr,
    uacpi_size,
    uacpi_status,
    uacpi_thread_id,
    uacpi_u8,
    uacpi_u16,
    uacpi_u32,
    uacpi_u64,
    uacpi_work_handler,
    uacpi_work_type,
};

use crate::{
    arch::x86_64::io,
    common::{boot, math},
    memory::{
        PAGE_SIZE,
        vmm::{self, Attribute},
    },
    sync::spinlock::SpinLock,
};

// Returns the PHYSICAL address of the RSDP structure via *out_rsdp_address.

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_get_rsdp(out_rsdp_address: *mut uacpi_phys_addr) -> uacpi_status {
    unsafe {
        *out_rsdp_address = boot::get_rsdp_address() - boot::get_hhdm_offset();
    }

    UACPI_STATUS_OK
}

/**
 * Map a physical memory range starting at 'addr' with length 'len', and return
 * a virtual address that can be used to access it.
 *
 * NOTE: 'addr' may be misaligned, in this case the host is expected to round it
 *       down to the nearest page-aligned boundary and map that, while making
 *       sure that at least 'len' bytes are still mapped starting at 'addr'. The
 *       return value preserves the misaligned offset.
 *
 *       Example for uacpi_kernel_map(0x1ABC, 0xF00):
 *           1. Round down the 'addr' we got to the nearest page boundary.
 *              Considering a PAGE_SIZE of 4096 (or 0x1000), 0x1ABC rounded down
 *              is 0x1000, offset within the page is 0x1ABC - 0x1000 => 0xABC
 *           2. Requested 'len' is 0xF00 bytes, but we just rounded the address
 *              down by 0xABC bytes, so add those on top. 0xF00 + 0xABC => 0x19BC
 *           3. Round up the final 'len' to the nearest PAGE_SIZE boundary, in
 *              this case 0x19BC is 0x2000 bytes (2 pages if PAGE_SIZE is 4096)
 *           4. Call the VMM to map the aligned address 0x1000 (from step 1)
 *              with length 0x2000 (from step 3). Let's assume the returned
 *              virtual address for the mapping is 0xF000.
 *           5. Add the original offset within page 0xABC (from step 1) to the
 *              resulting virtual address 0xF000 + 0xABC => 0xFABC. Return it
 *              to uACPI.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_map(addr: uacpi_phys_addr, len: uacpi_size) -> *mut c_void {
    let aligned_address = math::align_down(addr, PAGE_SIZE);
    let address_diff = addr - aligned_address;
    let aligned_length = math::align_up(len as u64 + address_diff, PAGE_SIZE);

    for offset in (0..aligned_length).step_by(PAGE_SIZE as usize) {
        vmm::map_into_kernel(aligned_address + offset, Attribute::WRITE);
    }

    (addr + boot::get_hhdm_offset()) as *mut c_void
}

/**
 * Unmap a virtual memory range at 'addr' with a length of 'len' bytes.
 *
 * NOTE: 'addr' may be misaligned, see the comment above 'uacpi_kernel_map'.
 *       Similar steps to uacpi_kernel_map can be taken to retrieve the
 *       virtual address originally returned by the VMM for this mapping
 *       as well as its true length.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_unmap(addr: *mut c_void, len: uacpi_size) {
    // TODO: Implement me
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_log(level: uacpi_log_level, string: *const uacpi_char) {
    let message = unsafe { CStr::from_ptr(string) }.to_str().unwrap();
    let trimmed_message = &message[0..message.len() - 1];

    match level {
        UACPI_LOG_INFO => log::info!("UACPI: {}", trimmed_message),
        UACPI_LOG_WARN => log::warn!("UACPI: {}", trimmed_message),
        UACPI_LOG_ERROR => log::error!("UACPI: {}", trimmed_message),
        _ => {}
    }
}

/**
 * Open a PCI device at 'address' for reading & writing.
 *
 * The device at 'address' might not actually exist on the system, in this case
 * the api is allowed to return UACPI_STATUS_NOT_FOUND to indicate that, this
 * error is handled gracefully by creating a dummy device internally that always
 * returns 0xFF on reads and is no-op for writes. This is to support a common
 * pattern in AML that probes for 0xFF reads to detect whether a device exists.
 *
 * The handle returned via 'out_handle' is used to perform IO on the
 * configuration space of the device.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_device_open(
    address: uacpi_pci_address,
    out_handle: *mut uacpi_handle,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_device_open");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_device_close(device: uacpi_handle) {
    log::warn!("UACPI: Implement uacpi_kernel_pci_device_close");
}

/**
 * Read & write the configuration space of a previously open PCI device.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_read8(
    device: uacpi_handle,
    offset: uacpi_size,
    value: *mut uacpi_u8,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_read8");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_read16(
    device: uacpi_handle,
    offset: uacpi_size,
    value: *mut uacpi_u16,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_read16");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_read32(
    device: uacpi_handle,
    offset: uacpi_size,
    value: *mut uacpi_u32,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_read32");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_write8(
    device: uacpi_handle,
    offset: uacpi_size,
    value: uacpi_u8,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_write8");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_write16(
    device: uacpi_handle,
    offset: uacpi_size,
    value: uacpi_u16,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_write16");
    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_pci_write32(
    device: uacpi_handle,
    offset: uacpi_size,
    value: uacpi_u32,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_pci_write32");
    UACPI_STATUS_OK
}

/**
 * Map a SystemIO address at [base, base + len) and return a kernel-implemented
 * handle that can be used for reading and writing the IO range.
 *
 * NOTE: The x86 architecture uses the in/out family of instructions
 *       to access the SystemIO address space.
*/

#[repr(C)]
struct IoMap {
    base: u64,
    length: u64,
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_map(
    base: uacpi_io_addr,
    len: uacpi_size,
    out_handle: *mut uacpi_handle,
) -> uacpi_status {
    let io_map = Box::new(IoMap {
        base,
        length: len as u64,
    });

    unsafe {
        *out_handle = Box::into_raw(io_map) as uacpi_handle;
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_unmap(handle: uacpi_handle) {
    if handle.is_null() {
        return;
    }

    drop(unsafe { Box::from_raw(handle as *mut IoMap) });
}

/**
 * Read/Write the IO range mapped via uacpi_kernel_io_map
 * at a 0-based 'offset' within the range.
 *
 * NOTE:
 * The x86 architecture uses the in/out family of instructions
 * to access the SystemIO address space.
 *
 * You are NOT allowed to break e.g. a 4-byte access into four 1-byte accesses.
 * Hardware ALWAYS expects accesses to be of the exact width.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_read8(
    handle: uacpi_handle,
    offset: uacpi_size,
    out_value: *mut uacpi_u8,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        *out_value = io::in8((io_map.base + offset as u64) as u16);
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_read16(
    handle: uacpi_handle,
    offset: uacpi_size,
    out_value: *mut uacpi_u16,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        *out_value = io::in16((io_map.base + offset as u64) as u16);
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_read32(
    handle: uacpi_handle,
    offset: uacpi_size,
    out_value: *mut uacpi_u32,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        *out_value = io::in32((io_map.base + offset as u64) as u16);
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_write8(
    handle: uacpi_handle,
    offset: uacpi_size,
    in_value: uacpi_u8,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        io::out8((io_map.base + offset as u64) as u16, in_value);
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_write16(
    handle: uacpi_handle,
    offset: uacpi_size,
    in_value: uacpi_u16,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        io::out16((io_map.base + offset as u64) as u16, in_value);
    }

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_io_write32(
    handle: uacpi_handle,
    offset: uacpi_size,
    in_value: uacpi_u32,
) -> uacpi_status {
    let io_map = unsafe { &*(handle as *const IoMap) };

    unsafe {
        io::out32((io_map.base + offset as u64) as u16, in_value);
    }

    UACPI_STATUS_OK
}

/**
 * Allocate a block of memory of 'size' bytes.
 * The contents of the allocated memory are unspecified.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_alloc(size: uacpi_size) -> *mut c_void {
    if size == 0 {
        return ptr::null_mut();
    }

    let ptr =
        unsafe { alloc::alloc::alloc(Layout::from_size_align(size, align_of::<usize>()).unwrap()) };
    ptr as *mut c_void
}

/**
 * Free a previously allocated memory block.
 *
 * 'mem' might be a NULL pointer. In this case, the call is assumed to be a
 * no-op.
 *
 * An optionally enabled 'size_hint' parameter contains the size of the original
 * allocation. Note that in some scenarios this incurs additional cost to
 * calculate the object size.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_free(mem: *mut c_void, size_hint: uacpi_size) {
    if mem.is_null() {
        return;
    }

    unsafe {
        alloc::alloc::dealloc(
            mem as *mut u8,
            Layout::from_size_align(size_hint, align_of::<usize>()).unwrap(),
        )
    };
}

/**
 * Returns the number of nanosecond ticks elapsed since boot,
 * strictly monotonic.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_get_nanoseconds_since_boot() -> uacpi_u64 {
    log::warn!("UACPI: Implement uacpi_kernel_get_nanoseconds_since_boot");
    0
}

/**
 * Spin for N microseconds.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_stall(usec: uacpi_u8) {
    log::warn!("UACPI: Implement uacpi_kernel_stall");
}

/**
 * Sleep for N milliseconds.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_sleep(msec: uacpi_u64) {
    log::warn!("UACPI: Implement uacpi_kernel_sleep");
}

/**
 * Create/free an opaque non-recursive kernel mutex object.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_create_mutex() -> uacpi_handle {
    let mutex = Box::new(SpinLock::new(()));
    Box::into_raw(mutex) as uacpi_handle
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_free_mutex(mutex: uacpi_handle) {
    if mutex.is_null() {
        return;
    }

    drop(unsafe { Box::from_raw(mutex as *mut SpinLock<()>) });
}

/**
 * Create/free an opaque kernel (semaphore-like) event object.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_create_event() -> uacpi_handle {
    log::warn!("UACPI: Implement uacpi_kernel_create_event");
    ptr::null_mut()
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_free_event(handle: uacpi_handle) {
    log::warn!("UACPI: Implement uacpi_kernel_free_event");
}

/**
 * Returns a unique identifier of the currently executing thread.
 *
 * The returned thread id cannot be UACPI_THREAD_ID_NONE.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_get_thread_id() -> uacpi_thread_id {
    // TODO: Implement me
    ptr::null_mut()
}

/**
 * Disable interrupts and return an kernel-defined value representing the
 * "before" state. This value is used in the subsequent call to restore the
 * prior state.
 *
 * Note that this is talking about ALL interrupts on the current CPU, not just
 * those installed by uACPI. This is typically achieved by executing the 'cli'
 * instruction on x86, 'msr daifset, #3' on aarch64 etc.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_disable_interrupts() -> uacpi_interrupt_state {
    log::warn!("UACPI: Implement uacpi_kernel_disable_interrupts");
    0
}

/**
 * Restore the state of the interrupt flags to the kernel-defined value provided
 * in 'state'.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_restore_interrupts(state: uacpi_interrupt_state) {
    log::warn!("UACPI: Implement uacpi_kernel_restore_interrupts");
}

/**
 * Try to acquire the mutex with a millisecond timeout.
 *
 * The timeout value has the following meanings:
 * 0x0000 - Attempt to acquire the mutex once, in a non-blocking manner
 * 0x0001...0xFFFE - Attempt to acquire the mutex for at least 'timeout'
 *                   milliseconds
 * 0xFFFF - Infinite wait, block until the mutex is acquired
 *
 * The following are possible return values:
 * 1. UACPI_STATUS_OK - successful acquire operation
 * 2. UACPI_STATUS_TIMEOUT - timeout reached while attempting to acquire (or the
 *                           single attempt to acquire was not successful for
 *                           calls with timeout=0)
 * 3. Any other value - signifies a host internal error and is treated as such
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_acquire_mutex(
    mutex: uacpi_handle,
    timeout: uacpi_u16,
) -> uacpi_status {
    if mutex.is_null() {
        return UACPI_STATUS_OK;
    }

    let mutex = unsafe { &*(mutex as *const SpinLock<()>) };
    mem::forget(mutex.lock());

    UACPI_STATUS_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_release_mutex(mutex: uacpi_handle) {
    if mutex.is_null() {
        return;
    }

    let mutex = unsafe { &*(mutex as *const SpinLock<()>) };
    unsafe { mutex.force_unlock() };
}

/**
 * Try to wait for an event (counter > 0) with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 *
 * The internal counter is decremented by 1 if wait was successful.
 *
 * A successful wait is indicated by returning UACPI_TRUE.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_wait_for_event(
    event: uacpi_handle,
    timeout: uacpi_u16,
) -> uacpi_bool {
    log::warn!("UACPI: Implement uacpi_kernel_wait_for_event");
    false
}

/**
 * Signal the event object by incrementing its internal counter by 1.
 *
 * This function may be used in interrupt contexts.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_signal_event(handle: uacpi_handle) {
    log::warn!("UACPI: Implement uacpi_kernel_signal_event");
}

/**
 * Reset the event counter to 0.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_reset_event(handle: uacpi_handle) {
    log::warn!("UACPI: Implement uacpi_kernel_reset_event");
}

/**
 * Handle a firmware request.
 *
 * Currently either a Breakpoint or Fatal operators.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_handle_firmware_request(
    request: *mut uacpi_firmware_request,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_handle_firmware_request");
    UACPI_STATUS_OK
}

/**
 * Install an interrupt handler at 'irq', 'ctx' is passed to the provided
 * handler for every invocation.
 *
 * 'out_irq_handle' is set to a kernel-implemented value that can be used to
 * refer to this handler from other API.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_install_interrupt_handler(
    irq: uacpi_u32,
    interrupt_handler: uacpi_interrupt_handler,
    ctx: uacpi_handle,
    out_irq_handle: *mut uacpi_handle,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_install_interrupt_handler");
    UACPI_STATUS_OK
}

/**
 * Uninstall an interrupt handler. 'irq_handle' is the value returned via
 * 'out_irq_handle' during installation.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_uninstall_interrupt_handler(
    interrupt_handler: uacpi_interrupt_handler,
    irq_handle: uacpi_handle,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_uninstall_interrupt_handler");
    UACPI_STATUS_OK
}

/**
 * Create/free a kernel spinlock object.
 *
 * Unlike other types of locks, spinlocks may be used in interrupt contexts.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_create_spinlock() -> uacpi_handle {
    let mutex = Box::new(SpinLock::new(()));
    Box::into_raw(mutex) as uacpi_handle
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_free_spinlock(spinlock: uacpi_handle) {
    if spinlock.is_null() {
        return;
    }

    drop(unsafe { Box::from_raw(spinlock as *mut SpinLock<()>) });
}

/**
 * Lock/unlock helpers for spinlocks.
 *
 * These are expected to disable interrupts, returning the previous state of cpu
 * flags, that can be used to possibly re-enable interrupts if they were enabled
 * before.
 *
 * Note that lock is infalliable.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_lock_spinlock(spinlock: uacpi_handle) -> uacpi_cpu_flags {
    if spinlock.is_null() {
        return 0;
    }

    let spinlock = unsafe { &*(spinlock as *const SpinLock<()>) };
    mem::forget(spinlock.lock());

    0
}

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_unlock_spinlock(spinlock: uacpi_handle, flags: uacpi_cpu_flags) {
    if spinlock.is_null() {
        return;
    }

    let spinlock = unsafe { &*(spinlock as *const SpinLock<()>) };
    unsafe { spinlock.force_unlock() };
}

/**
 * Schedules deferred work for execution.
 * Might be invoked from an interrupt context.
*/

#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_schedule_work(
    work_type: uacpi_work_type,
    work_handler: uacpi_work_handler,
    ctx: uacpi_handle,
) -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_schedule_work");
    UACPI_STATUS_OK
}

/**
 * Waits for two types of work to finish:
 * 1. All in-flight interrupts installed via uacpi_kernel_install_interrupt_handler
 * 2. All work scheduled via uacpi_kernel_schedule_work
 *
 * Note that the waits must be done in this order specifically.
*/
#[unsafe(no_mangle)]
unsafe extern "C" fn uacpi_kernel_wait_for_work_completion() -> uacpi_status {
    log::warn!("UACPI: Implement uacpi_kernel_wait_for_work_completion");
    UACPI_STATUS_OK
}
