/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/memory/vmm.hpp"

#include <lib/math.hpp>

#include "kernel/core/boot.hpp"
#include "kernel/core/logger.hpp"
#include "kernel/core/memory.hpp"
#include "kernel/memory/pmm.hpp"

namespace kernel::vmm {

static PageMap *s_kernel_page_map { nullptr };

extern "C" unsigned char __kernel_start[];
extern "C" unsigned char __kernel_end[];

void initialize()
{
    s_kernel_page_map = create_page_map();

    const usize memory_map_entry_count = boot::get_memory_map_entry_count();
    logger::debug("VMM: Mapping memory map entries...\n");

    usize mapped_entry_count = 0;
    usize mapped_bytes = 0;
    for (usize i = 0; i < memory_map_entry_count; ++i) {
        const limine_memmap_entry *entry = boot::get_memory_map_entry(i);

        if (entry->type != LIMINE_MEMMAP_USABLE && entry->type != LIMINE_MEMMAP_ACPI_RECLAIMABLE
            && entry->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE
            && entry->type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES && entry->type != LIMINE_MEMMAP_FRAMEBUFFER) {
            continue;
        }

        const usize entry_start = lib::math::align_down(entry->base, memory::s_page_size);
        const usize entry_end = lib::math::align_up(entry->base + entry->length, memory::s_page_size);
        const usize entry_pages = (entry_end - entry_start) / memory::s_page_size;

        logger::debug(
            "VMM:   [0x%016llx - 0x%016llx] -> [0x%016llx - 0x%016llx] (%zu pages)\n",
            entry_start,
            entry_end,
            entry_start + boot::get_hhdm_offset(),
            entry_end + boot::get_hhdm_offset(),
            entry_pages);

        for (usize j = entry_start; j < entry_end; j += memory::s_page_size) {
            map(s_kernel_page_map, j, j + boot::get_hhdm_offset(), Attribute::Write);
        }

        ++mapped_entry_count;
        mapped_bytes += entry_end - entry_start;
    }

    logger::debug(
        "VMM: Mapped %zu entries (%zu KiB, %zu MiB)\n",
        mapped_entry_count,
        mapped_bytes / 1024,
        mapped_bytes / 1024 / 1024);

    const usize kernel_virtual_start = lib::math::align_down(reinterpret_cast<usize>(&__kernel_start), memory::s_page_size);
    const usize kernel_virtual_end = lib::math::align_up(reinterpret_cast<usize>(&__kernel_end), memory::s_page_size);
    const usize physical_base = boot::get_executable_physical_base();
    const usize virtual_base = boot::get_executable_virtual_base();
    const usize kernel_pages = (kernel_virtual_end - kernel_virtual_start) / memory::s_page_size;

    logger::debug("VMM: Mapping kernel image...\n");
    logger::debug(
        "VMM:   [0x%016llx - 0x%016llx] -> [0x%016llx - 0x%016llx] (%zu pages)\n",
        kernel_virtual_start - virtual_base + physical_base,
        kernel_virtual_end - virtual_base + physical_base,
        kernel_virtual_start,
        kernel_virtual_end,
        kernel_pages);

    for (usize i = kernel_virtual_start; i < kernel_virtual_end; i += memory::s_page_size) {
        map(s_kernel_page_map, i - virtual_base + physical_base, i, Attribute::Write);
    }

    logger::debug("VMM: Mapped kernel (%zu KiB)\n", (kernel_virtual_end - kernel_virtual_start) / 1024);

    logger::debug("VMM: Switching to kernel page map...\n");
    switch_to_page_map(s_kernel_page_map);

    logger::info("VMM: Initialized\n");
}

PageMap *create_page_map()
{
    PageMap *page_map = new PageMap();
    page_map->top_level = reinterpret_cast<u64>(pmm::allocate(1, true));

    // NOTE: This copies the higher half of the kernel page map to every page map
    // FIXME: Find a good way to sync the kernel map if it changes to every other page map, a good way could be to do it
    // everytime a page map switch happens
    if (s_kernel_page_map) {
        u64 *new_pml4 = reinterpret_cast<u64 *>(page_map->top_level + boot::get_hhdm_offset());
        const u64 *kernel_pml4 = reinterpret_cast<u64 *>(s_kernel_page_map->top_level + boot::get_hhdm_offset());

        for (usize i = 256; i < 512; ++i) {
            new_pml4[i] = kernel_pml4[i];
        }
    }

    return page_map;
}

void switch_to_page_map(const PageMap *page_map)
{
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(static_cast<u64>(page_map->top_level)) : "memory");
}

static u64 get_next_level(const u64 pml, const u16 entry)
{
    u64 *pml_address = reinterpret_cast<u64 *>(pml + boot::get_hhdm_offset());
    u64 *pml_entry = &pml_address[entry];

    const Attribute attributes = static_cast<Attribute>(*pml_entry & 0xfff);
    if ((attributes & Attribute::Present) != Attribute::Present) {
        const u64 new_level = reinterpret_cast<u64>(pmm::allocate(1, true));
        *pml_entry = new_level | static_cast<u64>(Attribute::User | Attribute::Write | Attribute::Present);
    }

    // NOTE: Assuming MAXPHYADDR is 36, then generate mask and shift it by 12 bits for the flags
    return *pml_entry & (((1ull << 36) - 1) << 12);
}

static u64 *get_pte(const PageMap *page_map, const u64 vaddr)
{
    const u16 pml4_entry = (vaddr >> 39) & 0x1ff;
    const u64 pml4 = page_map->top_level;

    const u16 pdpt_entry = (vaddr >> 30) & 0x1ff;
    const u64 pdpt = get_next_level(pml4, pml4_entry);

    const u16 pd_entry = (vaddr >> 21) & 0x1ff;
    const u64 pd = get_next_level(pdpt, pdpt_entry);

    const u16 pt_entry = (vaddr >> 12) & 0x1ff;
    const u64 pt = get_next_level(pd, pd_entry);

    u64 *entry_address = reinterpret_cast<u64 *>(pt + boot::get_hhdm_offset());
    u64 *entry = &entry_address[pt_entry];

    return entry;
}

void map(const PageMap *page_map, const u64 paddr, const u64 vaddr, const Attribute attributes)
{
    const usize aligned_physical_address = lib::math::align_down(paddr, memory::s_page_size);
    const usize aligned_virtual_address = lib::math::align_down(vaddr, memory::s_page_size);

    u64 *entry = get_pte(page_map, aligned_virtual_address);
    *entry = aligned_physical_address | static_cast<u64>(attributes | Attribute::Present);
}

void unmap(const PageMap *page_map, const u64 vaddr)
{
    const usize aligned_virtual_address = lib::math::align_down(vaddr, memory::s_page_size);

    u64 *entry = get_pte(page_map, aligned_virtual_address);
    *entry = 0;
}

PageMap *get_kernel_page_map() { return s_kernel_page_map; }

} // namespace kernel::vmm
