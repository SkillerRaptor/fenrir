/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>

#include <uacpi/platform/arch_helpers.h>
#include <uacpi/status.h>
#include <uacpi/types.h>

#include "arch/x86_64/cpu.h"
#include "arch/x86_64/io.h"
#include "boot/boot.h"
#include "lib/log.h"
#include "lib/math.h"
#include "lib/stdio.h"
#include "mem/kmalloc.h"
#include "mem/page.h"
#include "mem/vmm.h"
#include "sync/semaphore.h"
#include "sync/spinlock.h"

// Returns the PHYSICAL address of the RSDP structure via *out_rsdp_address.
uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
    *out_rsdp_address = (uint64_t) boot_get_rsdp_address() - boot_get_hhdm_offset();
    return UACPI_STATUS_OK;
}

/**
 * Map a physical memory range starting at 'addr' with length 'len', and return
 * a virtual address that can be used to access it. UACPI_MAP_FAILED is returned
 * in case the operation wasn't successful.
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
void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
    const uint64_t aligned_address = math_align_down(addr, PAGE_SIZE);
    const uint64_t address_diff = addr - aligned_address;
    const uint64_t aligned_length = math_align_up(len + address_diff, PAGE_SIZE);

    const size_t pages = aligned_length / PAGE_SIZE;
    for (size_t i = 0; i < pages; ++i) {
        vmm_map_page_into_kernel(aligned_address + i * PAGE_SIZE, PAGE_ATTRIBUTE_WRITE);
    }

    return (void *) (addr + boot_get_hhdm_offset());
}

/**
 * Unmap a virtual memory range at 'addr' with a length of 'len' bytes.
 *
 * NOTE: 'addr' may be misaligned, see the comment above 'uacpi_kernel_map'.
 *       Similar steps to uacpi_kernel_map can be taken to retrieve the
 *       virtual address originally returned by the VMM for this mapping
 *       as well as its true length.
 */
void uacpi_kernel_unmap(void *addr, uacpi_size len)
{
    const uint64_t address = (uint64_t) addr;

    const uint64_t aligned_address = math_align_down(address, PAGE_SIZE);
    const uint64_t address_diff = address - aligned_address;
    const uint64_t aligned_length = math_align_up(len + address_diff, PAGE_SIZE);

    const size_t pages = aligned_length / PAGE_SIZE;
    for (size_t i = 0; i < pages; ++i) {
        vmm_unmap_page(vmm_get_kernel_page_map(), aligned_address + i * PAGE_SIZE + boot_get_hhdm_offset());
    }
}

UACPI_PRINTF_DECL(2, 3)
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *fmt, ...)
{
    enum log_level log_level = LOG_LEVEL_INFO;
    switch (level) {
    case UACPI_LOG_DEBUG:
        log_level = LOG_LEVEL_DEBUG;
        break;
    case UACPI_LOG_TRACE:
        log_level = LOG_LEVEL_TRACE;
        break;
    case UACPI_LOG_INFO:
        log_level = LOG_LEVEL_INFO;
        break;
    case UACPI_LOG_WARN:
        log_level = LOG_LEVEL_WARNING;
        break;
    case UACPI_LOG_ERROR:
        log_level = LOG_LEVEL_ERROR;
        break;
    }

    va_list args;
    va_start(args, fmt);
    log_vlog(log_level, fmt, args);
    va_end(args);
}

/**
 * Open a PCI device at 'address' for reading & writing.
 *
 * The device at 'address' might not actually exist on the system, in this case
 * the api is allowed to return UACPI_STATUS_NOT_FOUND to indicate that, this
 * error is handled gracefully by returning 0xFF on reads and ignoring writes.
 * This is to support a common pattern in AML that probes for 0xFF reads to
 * detect whether a device exists. The open is attempted again on the next
 * access to the device, to support hotplug & wake-from-suspend.
 *
 * The handle returned via 'out_handle' is used to perform IO on the
 * configuration space of the device.
 */
uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle)
{
    (void) address;
    (void) out_handle;

    LOG_TRACE("uacpi_kernel_pci_device_open()\n");

    return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle) { LOG_TRACE("uacpi_kernel_pci_device_close()\n"); }

/**
 * Read & write the configuration space of a previously open PCI device.
 */
uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_read8()\n");

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_read16()\n");

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_read32()\n");

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_write8()\n");

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_write16()\n");

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
    (void) device;
    (void) offset;
    (void) value;

    LOG_TRACE("uacpi_kernel_pci_write32()\n");

    return UACPI_STATUS_OK;
}

/**
 * Map a SystemIO address at [base, base + len) and return a kernel-implemented
 * handle that can be used for reading and writing the IO range.
 *
 * NOTE: The x86 architecture uses the in/out family of instructions
 *       to access the SystemIO address space.
 */

struct IoMap {
    uint64_t base;
    uint64_t length;
};

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle)
{
    struct IoMap *io_map = kmalloc(sizeof(struct IoMap));
    if (io_map == nullptr) {
        *out_handle = nullptr;
        return UACPI_STATUS_OUT_OF_MEMORY;
    }

    io_map->base = base;
    io_map->length = len;

    *out_handle = io_map;

    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle)
{
    struct IoMap *io_map = handle;
    kfree(io_map);
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
uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, uacpi_size offset, uacpi_u8 *out_value)
{
    struct IoMap *io_map = handle;
    *out_value = io_in8(io_map->base + offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset, uacpi_u16 *out_value)
{
    struct IoMap *io_map = handle;
    *out_value = io_in16(io_map->base + offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset, uacpi_u32 *out_value)
{
    struct IoMap *io_map = handle;
    *out_value = io_in32(io_map->base + offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, uacpi_size offset, uacpi_u8 in_value)
{
    struct IoMap *io_map = handle;
    io_out8(io_map->base + offset, in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, uacpi_size offset, uacpi_u16 in_value)
{
    struct IoMap *io_map = handle;
    io_out16(io_map->base + offset, in_value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, uacpi_size offset, uacpi_u32 in_value)
{
    struct IoMap *io_map = handle;
    io_out32(io_map->base + offset, in_value);
    return UACPI_STATUS_OK;
}

/**
 * Read/write to previously mapped memory
 *
 * If UACPI_NATIVE_MMIO is not defined, uACPI uses builtin generic MMIO
 * helpers that use simple volatile stores/loads.
 */
#ifdef UACPI_NATIVE_MMIO
uacpi_u8 uacpi_kernel_mmio_read8(void *);
uacpi_u16 uacpi_kernel_mmio_read16(void *);
uacpi_u32 uacpi_kernel_mmio_read32(void *);
uacpi_u64 uacpi_kernel_mmio_read64(void *);

void uacpi_kernel_mmio_write8(void *, uacpi_u8);
void uacpi_kernel_mmio_write16(void *, uacpi_u16);
void uacpi_kernel_mmio_write32(void *, uacpi_u32);
void uacpi_kernel_mmio_write64(void *, uacpi_u64);
#endif

/**
 * Allocate a block of memory of 'size' bytes.
 * The contents of the allocated memory are unspecified.
 */
void *uacpi_kernel_alloc(uacpi_size size) { return kmalloc(size); }

/**
 * Allocate a block of memory of 'size' bytes.
 * The returned memory block is expected to be zero-filled.
 */
void *uacpi_kernel_alloc_zeroed(uacpi_size size) { return kcalloc(1, size); }

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
void uacpi_kernel_free(void *mem) { kfree(mem); }

/**
 * Returns the number of nanosecond ticks elapsed since boot,
 * strictly monotonic.
 */
uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{
    LOG_TRACE("uacpi_kernel_get_nanoseconds_since_boot()\n");

    return 0;
}

/**
 * Spin for N microseconds.
 */
void uacpi_kernel_stall(uacpi_u8 usec)
{
    (void) usec;

    LOG_TRACE("uacpi_kernel_stall()\n");
}

/**
 * Sleep for N milliseconds.
 */
void uacpi_kernel_sleep(uacpi_u64 msec)
{
    (void) msec;

    LOG_TRACE("uacpi_kernel_sleep()\n");
}

/**
 * Create/free an opaque kernel counting semaphore object that starts out with
 * 'initial_units' units available.
 *
 * This is the only blocking synchronization primitive that uACPI relies upon,
 * it's used for two things:
 * - As a lock, in which case it's created with 1 unit, and never has more
 *   than that.
 * - As an event, in which case it's created with 0 units, and there's no
 *   limit on how many of them it may accumulate. A kernel whose semaphore
 *   requires one should use the largest value that it supports.
 *
 * A semaphore is never freed while there's a thread waiting on it, as long as
 * uacpi_state_reset is not called with another thread still inside of uACPI.
 */
uacpi_handle uacpi_kernel_create_semaphore(uacpi_u32 initial_units)
{
    struct Semaphore *semaphore = kmalloc(sizeof(struct Semaphore));
    semaphore->counter = initial_units;

    return semaphore;
}

void uacpi_kernel_free_semaphore(uacpi_handle handle)
{
    struct Semaphore *semaphore = handle;
    kfree(semaphore);
}

/**
 * Returns a unique identifier of the currently executing thread.
 *
 * The returned thread id cannot be UACPI_THREAD_ID_NONE.
 */
uacpi_thread_id uacpi_kernel_get_thread_id(void)
{
    // TODO: Implement real thread id
    return (uacpi_thread_id) 0;
}

/**
 * Disable interrupts and return a kernel-defined value representing the
 * "before" state. This value is used in the subsequent call to restore the
 * prior state.
 *
 * Note that this is talking about ALL interrupts on the current CPU, not just
 * those installed by uACPI. This is typically achieved by executing the 'cli'
 * instruction on x86, 'msr daifset, #3' on aarch64 etc.
 */
uacpi_interrupt_state uacpi_kernel_disable_interrupts(void)
{
    const bool interrupts_enabled = cpu_are_interrupts_enabled();
    cpu_disable_interrupts();
    return (uacpi_interrupt_state) interrupts_enabled;
}

/**
 * Restore the state of the interrupt flags to the kernel-defined value provided
 * in 'state'.
 */
void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state)
{
    if ((bool) state) {
        cpu_enable_interrupts();
    }
}

/**
 * Try to take one unit from the semaphore with a millisecond timeout.
 *
 * The timeout value has the following meanings:
 * 0x0000 - Attempt to take a unit once, in a non-blocking manner
 * 0x0001...0xFFFE - Attempt to take a unit for at least 'timeout' milliseconds
 * 0xFFFF - Infinite wait, block until a unit is available
 *
 * The following are possible return values:
 * 1. UACPI_STATUS_OK - a unit was taken
 * 2. UACPI_STATUS_TIMEOUT - timeout reached while waiting for a unit (or the
 *                           single attempt to take one was not successful for
 *                           calls with timeout=0)
 * 3. Any other value - signifies a host internal error and is treated as such
 *
 * This function is never used in interrupt contexts, unless that's where the
 * kernel chooses to execute deferred work, see uacpi_kernel_schedule_work.
 */
uacpi_status uacpi_kernel_wait_for_semaphore(uacpi_handle, uacpi_u16)
{
    // TODO: Implement uacpi_kernel_wait_for_semaphore

    return UACPI_STATUS_OK;
}

/**
 * Give one unit to the semaphore, waking up one of the threads that are
 * waiting for it, if any.
 *
 * A semaphore has no notion of an owner: the unit may be given by a thread
 * other than the one that took it.
 *
 * This function may be used in interrupt contexts, as well as with interrupts
 * disabled and a spinlock held, and therefore must not sleep. Note that this
 * applies to every semaphore, including those that are used as locks.
 */
void uacpi_kernel_signal_semaphore(uacpi_handle handle)
{
    struct Semaphore *semaphore = handle;

    spinlock_lock(&semaphore->lock);
    ++semaphore->counter;
    spinlock_unlock(&semaphore->lock);
}

/**
 * Handle a firmware request.
 *
 * Currently either a Breakpoint or Fatal operators.
 */
uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *)
{
    LOG_TRACE("uacpi_kernel_handle_firmware_request()\n");

    return UACPI_STATUS_OK;
}

/**
 * Install an interrupt handler at 'irq', 'ctx' is passed to the provided
 * handler for every invocation.
 *
 * 'out_irq_handle' is set to a kernel-implemented value that can be used to
 * refer to this handler from other API.
 */
uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx, uacpi_handle *out_irq_handle)
{
    (void) irq;
    (void) ctx;
    (void) out_irq_handle;

    LOG_TRACE("uacpi_kernel_install_interrupt_handler()\n");

    return UACPI_STATUS_OK;
}

/**
 * Uninstall an interrupt handler. 'irq_handle' is the value returned via
 * 'out_irq_handle' during installation.
 */
uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler, uacpi_handle irq_handle)
{
    (void) irq_handle;

    LOG_TRACE("uacpi_kernel_uninstall_interrupt_handler()\n");

    return UACPI_STATUS_OK;
}

/**
 * Create/free a kernel spinlock object.
 *
 * Unlike other types of locks, spinlocks may be used in interrupt contexts.
 */
uacpi_handle uacpi_kernel_create_spinlock(void)
{
    struct Spinlock *spinlock = kmalloc(sizeof(struct Spinlock));
    return spinlock;
}

void uacpi_kernel_free_spinlock(uacpi_handle handle)
{
    struct Spinlock *spinlock = handle;
    kfree(spinlock);
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
uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle)
{
    // TODO: Handle CPU Flags

    struct Spinlock *spinlock = handle;
    spinlock_lock(spinlock);

    return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags)
{
    // TODO: Handle CPU Flags

    struct Spinlock *spinlock = handle;
    spinlock_unlock(spinlock);
}

typedef enum uacpi_work_type {
    /**
     * Schedule a GPE handler method for execution.
     * This should be scheduled to run on CPU0 to avoid potential SMI-related
     * firmware bugs.
     */
    UACPI_WORK_GPE_EXECUTION,

    /**
     * Schedule a Notify(device) firmware request for execution.
     * This can run on any CPU.
     */
    UACPI_WORK_NOTIFICATION,
} uacpi_work_type;

typedef void (*uacpi_work_handler)(uacpi_handle);

/**
 * Create/free an opaque kernel work item object, which is what a piece of
 * deferred work is scheduled with, see uacpi_kernel_schedule_work.
 *
 * A work item is always created ahead of the time that it's needed, so that
 * scheduling it doesn't require any memory to be allocated. It might end up
 * never being scheduled at all.
 *
 * A kernel that has no use for a work item, e.g. because it executes all of
 * the work right away, may return any value other than UACPI_NULL without
 * allocating anything.
 *
 * Both of these might be invoked by the deferred work itself, but are never
 * used in interrupt contexts, unless that's where the kernel chooses to
 * execute said work.
 */
uacpi_handle uacpi_kernel_create_work_item(void)
{
    LOG_TRACE("uacpi_kernel_create_work_item()\n");

    return nullptr;
}

void uacpi_kernel_free_work_item(uacpi_handle) { LOG_TRACE("uacpi_kernel_free_work_item()\n"); }

/**
 * Schedules 'handler' to be invoked with 'ctx' as its only argument.
 * Might be invoked from an interrupt context.
 *
 * 'work_item' is there for the kernel to keep track of this work with, it's
 * never scheduled while still pending, that is before the handler that it was
 * last scheduled with has been invoked.
 *
 * Once that happens the work item may be reused: the handler is allowed to
 * schedule it again, possibly as a different type of work, or to free it. The
 * kernel therefore must not access a work item after it has invoked the
 * handler, which means that both 'handler' and 'ctx' have to be taken out of
 * it, and any of its state updated, before the handler is invoked and not
 * after it returns.
 */
void uacpi_kernel_schedule_work(uacpi_work_type, uacpi_handle work_item, uacpi_work_handler handler, uacpi_handle ctx)
{
    (void) work_item;
    (void) handler;
    (void) ctx;

    LOG_TRACE("uacpi_kernel_schedule_work()\n");
}

/**
 * Waits for two types of work to finish:
 * 1. All in-flight interrupts installed via uacpi_kernel_install_interrupt_handler
 * 2. All work scheduled via uacpi_kernel_schedule_work, including whatever
 *    was scheduled by the work that is being waited for
 *
 * Note that the waits must be done in this order specifically.
 *
 * uACPI never calls this from an interrupt handler or from deferred work, so
 * it's free to sleep, and never ends up waiting for its own caller. This
 * relies on the host not calling any API that is documented as off limits to
 * those from there.
 *
 * NOTE: if any of the work is executed synchronously, this must wait for the
 *       work that another thread is in the middle of executing that way too.
 */
uacpi_status uacpi_kernel_wait_for_work_completion(void)
{
    LOG_TRACE("uacpi_kernel_wait_for_work_completion()\n");

    return UACPI_STATUS_OK;
}
