/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "mem/vmm.h"

#include <stdint.h>

#include "boot/boot.h"
#include "lib/assert.h"
#include "lib/log.h"
#include "lib/math.h"
#include "limine.h"
#include "mem/kmalloc.h"
#include "mem/page.h"
#include "mem/pmm.h"

extern unsigned char __kernel_start[];
extern unsigned char __kernel_end[];

static constexpr uint64_t ADDRESS_MASK = ((1ull << 36) - 1) << 12;

static struct PageMap *s_kernel_page_map = nullptr;

void vmm_init()
{
    s_kernel_page_map = vmm_create_page_map();

    const struct limine_memmap_response *memory_map_response = boot_get_memory_map_response();

    size_t mapped_entry_count = 0;
    size_t mapped_bytes = 0;
    for (size_t i = 0; i < memory_map_response->entry_count; ++i) {
        const struct limine_memmap_entry *entry = memory_map_response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE && entry->type != LIMINE_MEMMAP_ACPI_RECLAIMABLE
            && entry->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE
            && entry->type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES && entry->type != LIMINE_MEMMAP_FRAMEBUFFER) {
            continue;
        }

        const uint64_t entry_start = math_align_down(entry->base, PAGE_SIZE);
        const uint64_t entry_end = math_align_up(entry->base + entry->length, PAGE_SIZE);

        const size_t entry_size = entry_end - entry_start;
        const size_t pages = math_div_round_up(entry_size, PAGE_SIZE);

        for (size_t i = 0; i < pages; ++i) {
            const uint64_t page_address = entry_start + i * PAGE_SIZE;
            vmm_map_page(s_kernel_page_map, page_address, page_address + boot_get_hhdm_offset(), PAGE_ATTRIBUTE_WRITE);
        }

        ++mapped_entry_count;
        mapped_bytes += entry_size;
    }

    LOG_DEBUG("VMM: Mapped %lu entries (%lu MiB)\n", mapped_entry_count, mapped_bytes / (1024 * 1024));

    const uint64_t kernel_virtual_start = math_align_down((uint64_t) &__kernel_start, PAGE_SIZE);
    const uint64_t kernel_virtual_end = math_align_up((uint64_t) &__kernel_end, PAGE_SIZE);

    const uint64_t physical_base = boot_get_executable_physical_base();
    const uint64_t virtual_base = boot_get_executable_virtual_base();

    const size_t kernel_size = kernel_virtual_end - kernel_virtual_start;
    const size_t kernel_pages = math_div_round_up(kernel_size, PAGE_SIZE);

    LOG_DEBUG("VMM: Mapping kernel image\n");
    LOG_DEBUG(
        "VMM:   [%#018lx-%#018lx] -> [%#018lx-%#018lx] (%lu pages)\n",
        kernel_virtual_start - virtual_base + physical_base,
        kernel_virtual_end - virtual_base + physical_base,
        kernel_virtual_start,
        kernel_virtual_end,
        kernel_pages);

    for (size_t i = 0; i < kernel_pages; ++i) {
        const uint64_t page_address = kernel_virtual_start + i * PAGE_SIZE;
        vmm_map_page(
            s_kernel_page_map,
            page_address - virtual_base + physical_base,
            page_address,
            PAGE_ATTRIBUTE_WRITE);
    }

    LOG_DEBUG("VMM: Mapped kernel (%lu KiB)\n", kernel_size / 1024)

    LOG_DEBUG("VMM: Switching to kernel page map\n");
    vmm_switch_page_map(s_kernel_page_map);

    LOG_INFO("VMM: Initialized\n");
}

struct PageMap *vmm_create_page_map()
{
    struct PageMap *page_map = kmalloc(sizeof(struct PageMap));
    if (!page_map) {
        return nullptr;
    }

    page_map->top_level = pmm_calloc(1);
    if (page_map->top_level == nullptr) {
        kfree(page_map);
        return nullptr;
    }

    if (s_kernel_page_map) {
        uint64_t *top_level = (uint64_t *) ((uint64_t) page_map->top_level + boot_get_hhdm_offset());
        const uint64_t *kernel_top_level
            = (const uint64_t *) ((uint64_t) s_kernel_page_map->top_level + boot_get_hhdm_offset());

        for (size_t i = 256; i < 512; ++i) {
            if (kernel_top_level[i] & PAGE_ATTRIBUTE_PRESENT) {
                top_level[i] = kernel_top_level[i];
            }
        }
    }

    return page_map;
}

void vmm_switch_page_map(struct PageMap *page_map)
{
    ASSERT(page_map != nullptr);
    ASSERT(page_map->top_level != nullptr);
    ASSERT(math_is_aligned((uint64_t) page_map->top_level, PAGE_SIZE));

    asm volatile("mov %0, %%cr3" : : "r"(page_map->top_level) : "memory");
}

struct PageMap *vmm_get_kernel_page_map() { return s_kernel_page_map; }

static uint64_t get_next_level(uint64_t pml, uint16_t entry, bool create)
{
    uint64_t *pml_entry_address = (uint64_t *) (pml + boot_get_hhdm_offset());
    uint64_t *pml_entry = &pml_entry_address[entry];

    if ((*pml_entry & PAGE_ATTRIBUTE_PRESENT) == 0) {
        if (!create) {
            return 0;
        }

        const uint64_t new_level = (uint64_t) pmm_calloc(1);
        if (new_level == 0) {
            return 0;
        }

        *pml_entry = new_level | PAGE_ATTRIBUTE_USER | PAGE_ATTRIBUTE_WRITE | PAGE_ATTRIBUTE_PRESENT;
    }

    return *pml_entry & ADDRESS_MASK;
}

static uint64_t *get_pte(struct PageMap *page_map, uintptr_t virt_addr, bool create)
{
    const uint16_t pml4_entry = (virt_addr >> 39) & 0x1ff;
    const uint64_t pml4 = (uint64_t) page_map->top_level;

    const uint16_t pdpt_entry = (virt_addr >> 30) & 0x1ff;
    const uint64_t pdpt = get_next_level(pml4, pml4_entry, create);
    if (pdpt == 0) {
        return nullptr;
    }

    const uint16_t pd_entry = (virt_addr >> 21) & 0x1ff;
    const uint64_t pd = get_next_level(pdpt, pdpt_entry, create);
    if (pdpt == 0) {
        return nullptr;
    }

    const uint16_t pt_entry = (virt_addr >> 12) & 0x1ff;
    const uint64_t pt = get_next_level(pd, pd_entry, create);
    if (pt == 0) {
        return nullptr;
    }

    uint64_t *entry_address = (uint64_t *) (pt + boot_get_hhdm_offset());
    return &entry_address[pt_entry];
}

static void invalidate_page(uintptr_t virt_addr) { asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory"); }

void vmm_map_page(struct PageMap *page_map, uint64_t phys_addr, uint64_t virt_addr, enum PageAttribute attributes)
{
    ASSERT(page_map != nullptr);
    ASSERT(page_map->top_level != nullptr);
    ASSERT(math_is_aligned(phys_addr, PAGE_SIZE));
    ASSERT(math_is_aligned(virt_addr, PAGE_SIZE));

    volatile uint64_t *pte = get_pte(page_map, virt_addr, true);
    if (!pte) {
        return;
    }

    *pte = (phys_addr & ADDRESS_MASK) | attributes | PAGE_ATTRIBUTE_PRESENT;

    invalidate_page(virt_addr);
}

void vmm_unmap_page(struct PageMap *page_map, uint64_t virt_addr)
{
    ASSERT(page_map != nullptr);
    ASSERT(page_map->top_level != nullptr);
    ASSERT(math_is_aligned(virt_addr, PAGE_SIZE));

    volatile uint64_t *pte = get_pte(page_map, virt_addr, false);
    if (!pte) {
        return;
    }

    *pte = 0;

    invalidate_page(virt_addr);
}
