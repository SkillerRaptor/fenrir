/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/acpi/acpi.hpp"

#include <lib/math.hpp>
#include <uacpi/event.h>
#include <uacpi/uacpi.h>

#include "kernel/arch/x86_64/io.hpp"
#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/memory.hpp"
#include "kernel/memory/kmalloc.hpp"
#include "kernel/memory/vmm.hpp"
#include "kernel/sync/spinlock.hpp"

namespace kernel::acpi {

void initialize()
{
    uacpi_status ret = uacpi_initialize(0);
    if (uacpi_unlikely_error(ret)) {
        logger::err("uacpi_initialize error: %s\n", uacpi_status_to_string(ret));
    }

    // ret = uacpi_namespace_load();
    // if (uacpi_unlikely_error(ret)) {
    //     logger::err("uacpi_namespace_load error: %s\n", uacpi_status_to_string(ret));
    // }

    // ret = uacpi_namespace_initialize();
    // if (uacpi_unlikely_error(ret)) {
    //     logger::err("uacpi_namespace_initialize error: %s\n", uacpi_status_to_string(ret));
    // }

    // ret = uacpi_finalize_gpe_initialization();
    // if (uacpi_unlikely_error(ret)) {
    //     logger::err("uacpi_finalize_gpe_initialization error: %s\n", uacpi_status_to_string(ret));
    // }

    logger::info("ACPI: Initialized\n");
}

} // namespace kernel::acpi

extern "C" {

// Returns the PHYSICAL address of the RSDP structure via *out_rsdp_address.
uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
    *out_rsdp_address
        = reinterpret_cast<uacpi_phys_addr>(kernel::boot::get_rsdp_address()) - kernel::boot::get_hhdm_offset();
    return UACPI_STATUS_OK;
}

/*
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
void *uacpi_kernel_map(const uacpi_phys_addr addr, const uacpi_size len)
{
    const u64 aligned_address = lib::math::align_down(addr, kernel::memory::s_page_size);
    const u64 address_diff = addr - aligned_address;
    const u64 aligned_length = lib::math::align_up(len + address_diff, kernel::memory::s_page_size);

    for (usize i = 0; i < aligned_length; i += kernel::memory::s_page_size) {
        kernel::vmm::map(
            kernel::vmm::get_kernel_page_map(),
            aligned_address + i,
            aligned_address + i + kernel::boot::get_hhdm_offset(),
            kernel::vmm::Attribute::Write);
    }

    return reinterpret_cast<void *>(addr + kernel::boot::get_hhdm_offset());
}

/*
 * Unmap a virtual memory range at 'addr' with a length of 'len' bytes.
 *
 * NOTE: 'addr' may be misaligned, see the comment above 'uacpi_kernel_map'.
 *       Similar steps to uacpi_kernel_map can be taken to retrieve the
 *       virtual address originally returned by the VMM for this mapping
 *       as well as its true length.
 */
void uacpi_kernel_unmap(void *addr, const uacpi_size len)
{
    const u64 virtual_address = reinterpret_cast<u64>(addr);
    const u64 aligned_address = lib::math::align_down(virtual_address, kernel::memory::s_page_size);
    const u64 address_diff = virtual_address - aligned_address;
    const u64 aligned_length = lib::math::align_up(len + address_diff, kernel::memory::s_page_size);

    for (usize i = 0; i < aligned_length; i += kernel::memory::s_page_size) {
        kernel::vmm::unmap(kernel::vmm::get_kernel_page_map(), aligned_address + i);
    }
}

#ifndef UACPI_FORMATTED_LOGGING
void uacpi_kernel_log(const uacpi_log_level level, const uacpi_char *str)
{
    switch (level) {
    case UACPI_LOG_INFO:
        kernel::logger::debug("UACPI: %s", str);
        break;
    case UACPI_LOG_WARN:
        kernel::logger::warn("UACPI: %s", str);
        break;
    case UACPI_LOG_ERROR:
        kernel::logger::err("UACPI: %s", str);
        break;
    default:
        break;
    }
}
#else
UACPI_PRINTF_DECL(2, 3)
void uacpi_kernel_log(uacpi_log_level, const uacpi_char *, ...);
void uacpi_kernel_vlog(uacpi_log_level, const uacpi_char *, uacpi_va_list);
#endif

/*
 * Only the above ^^^ API may be used by early table access and
 * UACPI_BAREBONES_MODE.
 */
#ifndef UACPI_BAREBONES_MODE

/*
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
uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle)
{
    kernel::logger::warn("uacpi_kernel_pci_device_open not implemented!\n");
    return UACPI_STATUS_OK;
}
void uacpi_kernel_pci_device_close(uacpi_handle)
{
    kernel::logger::warn("uacpi_kernel_pci_device_close not implemented!\n");
}

/*
 * Read & write the configuration space of a previously open PCI device.
 */
uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *value)
{
    kernel::logger::warn("uacpi_kernel_pci_read8 not implemented!\n");
    return UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *value)
{
    kernel::logger::warn("uacpi_kernel_pci_read16 not implemented!\n");
    return UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *value)
{
    kernel::logger::warn("uacpi_kernel_pci_read32 not implemented!\n");
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
    kernel::logger::warn("uacpi_kernel_pci_write8 not implemented!\n");
    return UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
    kernel::logger::warn("uacpi_kernel_pci_write16 not implemented!\n");
    return UACPI_STATUS_OK;
}
uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
    kernel::logger::warn("uacpi_kernel_pci_write32 not implemented!\n");
    return UACPI_STATUS_OK;
}

/*
 * Map a SystemIO address at [base, base + len) and return a kernel-implemented
 * handle that can be used for reading and writing the IO range.
 *
 * NOTE: The x86 architecture uses the in/out family of instructions
 *       to access the SystemIO address space.
 */

struct IOMap {
    u64 base { 0 };
    u64 length { 0 };
} __attribute__((packed));

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, const uacpi_size len, uacpi_handle *out_handle)
{
    IOMap *io_map = new IOMap();
    io_map->base = base;
    io_map->length = len;
    *out_handle = io_map;

    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) { delete static_cast<IOMap *>(handle); }

/*
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
uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, const uacpi_size offset, uacpi_u8 *out_value)
{
    *out_value = kernel::io::in8(*(static_cast<u16 *>(handle) + offset));
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset, uacpi_u16 *out_value)
{
    *out_value = kernel::io::in16(*(static_cast<u16 *>(handle) + offset));
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset, uacpi_u32 *out_value)
{
    *out_value = kernel::io::in32(*(static_cast<u16 *>(handle) + offset));
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, const uacpi_size offset, const uacpi_u8 in_value)
{
    kernel::io::out8(*(static_cast<u16 *>(handle) + offset), in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, const uacpi_size offset, const uacpi_u16 in_value)
{
    kernel::io::out16(*(static_cast<u16 *>(handle) + offset), in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, const uacpi_size offset, const uacpi_u32 in_value)
{
    kernel::io::out32(*(static_cast<u16 *>(handle) + offset), in_value);
    return UACPI_STATUS_OK;
}

/*
 * Allocate a block of memory of 'size' bytes.
 * The contents of the allocated memory are unspecified.
 */
void *uacpi_kernel_alloc(const uacpi_size size) { return kernel::memory::kmalloc(size); }

/*
 * Free a previously allocated memory block.
 *
 * 'mem' might be a NULL pointer. In this case, the call is assumed to be a
 * no-op.
 *
 * An optionally enabled 'size_hint' parameter contains the size of the original
 * allocation. Note that in some scenarios this incurs additional cost to
 * calculate the object size.
 */
void uacpi_kernel_free(void *mem) { kernel::memory::kfree(mem); }

/*
 * Returns the number of nanosecond ticks elapsed since boot,
 * strictly monotonic.
 */
uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot()
{
    kernel::logger::warn("uacpi_kernel_get_nanoseconds_since_boot not implemented!\n");
    return 0;
}

/*
 * Spin for N microseconds.
 */
void uacpi_kernel_stall(uacpi_u8 usec) { kernel::logger::warn("uacpi_kernel_stall not implemented!\n"); }

/*
 * Sleep for N milliseconds.
 */
void uacpi_kernel_sleep(uacpi_u64 msec) { kernel::logger::warn("uacpi_kernel_sleep not implemented!\n"); }

/*
 * Create/free an opaque non-recursive kernel mutex object.
 */
uacpi_handle uacpi_kernel_create_mutex() { return new kernel::Spinlock(); }

void uacpi_kernel_free_mutex(uacpi_handle handle) { delete static_cast<kernel::Spinlock *>(handle); }

/*
 * Create/free an opaque kernel ( semaphore-like) event object.
 */
uacpi_handle uacpi_kernel_create_event() { return new int; }

void uacpi_kernel_free_event(uacpi_handle handle) { delete static_cast<int *>(handle); }

/*
 * Returns a unique identifier of the currently executing thread.
 *
 * The returned thread id cannot be UACPI_THREAD_ID_NONE.
 */
uacpi_thread_id uacpi_kernel_get_thread_id()
{
    // TODO: Replace this with the TID
    return reinterpret_cast<uacpi_thread_id>(0);
}

/*
 * Disable interrupts and return an kernel-defined value representing the
 * "before" state. This value is used in the subsequent call to restore the
 * prior state.
 *
 * Note that this is talking about ALL interrupts on the current CPU, not just
 * those installed by uACPI. This is typically achieved by executing the 'cli'
 * instruction on x86, 'msr daifset, #3' on aarch64 etc.
 */
uacpi_interrupt_state uacpi_kernel_disable_interrupts()
{
    kernel::logger::warn("uacpi_kernel_disable_interrupts not implemented!\n");
    return 0;
}

/*
 * Restore the state of the interrupt flags to the kernel-defined value provided
 * in 'state'.
 */
void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state)
{
    kernel::logger::warn("uacpi_kernel_restore_interrupts not implemented!\n");
}

/*
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
uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16)
{
    kernel::Spinlock *spinlock = static_cast<kernel::Spinlock *>(handle);
    spinlock->lock();

    return UACPI_STATUS_OK;
}

void uacpi_kernel_release_mutex(uacpi_handle handle)
{
    kernel::Spinlock *spinlock = static_cast<kernel::Spinlock *>(handle);
    spinlock->unlock();
}

/*
 * Try to wait for an event (counter > 0) with a millisecond timeout.
 * A timeout value of 0xFFFF implies infinite wait.
 *
 * The internal counter is decremented by 1 if wait was successful.
 *
 * A successful wait is indicated by returning UACPI_TRUE.
 */
uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16)
{
    kernel::logger::warn("uacpi_kernel_wait_for_event not implemented!\n");
    return UACPI_FALSE;
}

/*
 * Signal the event object by incrementing its internal counter by 1.
 *
 * This function may be used in interrupt contexts.
 */
void uacpi_kernel_signal_event(uacpi_handle) { kernel::logger::warn("uacpi_kernel_signal_event not implemented!\n"); }

/*
 * Reset the event counter to 0.
 */
void uacpi_kernel_reset_event(uacpi_handle) { kernel::logger::warn("uacpi_kernel_reset_event not implemented!\n"); }

/*
 * Handle a firmware request.
 *
 * Currently either a Breakpoint or Fatal operators.
 */
uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *)
{
    kernel::logger::warn("uacpi_kernel_handle_firmware_request not implemented!\n");
    return UACPI_STATUS_OK;
}

/*
 * Install an interrupt handler at 'irq', 'ctx' is passed to the provided
 * handler for every invocation.
 *
 * 'out_irq_handle' is set to a kernel-implemented value that can be used to
 * refer to this handler from other API.
 */
uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx, uacpi_handle *out_irq_handle)
{
    kernel::logger::warn("uacpi_kernel_install_interrupt_handler not implemented!\n");
    return UACPI_STATUS_OK;
}

/*
 * Uninstall an interrupt handler. 'irq_handle' is the value returned via
 * 'out_irq_handle' during installation.
 */
uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler, uacpi_handle irq_handle)
{
    kernel::logger::warn("uacpi_kernel_uninstall_interrupt_handler not implemented!\n");
    return UACPI_STATUS_OK;
}

/*
 * Create/free a kernel spinlock object.
 *
 * Unlike other types of locks, spinlocks may be used in interrupt contexts.
 */
uacpi_handle uacpi_kernel_create_spinlock() { return new kernel::Spinlock(); }

void uacpi_kernel_free_spinlock(uacpi_handle handle) { delete static_cast<kernel::Spinlock *>(handle); }

/*
 * Lock/unlock helpers for spinlocks.
 *
 * These are expected to disable interrupts, returning the previous state of cpu
 * flags, that can be used to possibly re-enable interrupts if they were enabled
 * before.
 *
 * Note that lock is infalliable.
 */
uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle)
{
    kernel::Spinlock *spinlock = static_cast<kernel::Spinlock *>(handle);
    spinlock->lock();

    return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags)
{
    kernel::Spinlock *spinlock = static_cast<kernel::Spinlock *>(handle);
    spinlock->unlock();
}

/*
 * Schedules deferred work for execution.
 * Might be invoked from an interrupt context.
 */
uacpi_status uacpi_kernel_schedule_work(uacpi_work_type, uacpi_work_handler, uacpi_handle ctx)
{
    kernel::logger::warn("uacpi_kernel_schedule_work not implemented!\n");
    return UACPI_STATUS_OK;
}

/*
 * Waits for two types of work to finish:
 * 1. All in-flight interrupts installed via uacpi_kernel_install_interrupt_handler
 * 2. All work scheduled via uacpi_kernel_schedule_work
 *
 * Note that the waits must be done in this order specifically.
 */
uacpi_status uacpi_kernel_wait_for_work_completion()
{
    kernel::logger::warn("uacpi_kernel_wait_for_work_completion not implemented!\n");
    return UACPI_STATUS_OK;
}

#endif // !UACPI_BAREBONES_MODE

} // extern "C"
