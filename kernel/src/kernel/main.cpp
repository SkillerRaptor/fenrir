/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <limine.h>

#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/arch/x86_64/gdt.hpp"
#include "kernel/arch/x86_64/idt.hpp"
#include "kernel/core/types.hpp"
#include "kernel/misc/cxxabi.hpp"

namespace kernel {

__attribute__((used, section(".limine_requests"))) volatile u64 s_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request s_framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

__attribute__((used, section(".limine_requests_start"))) volatile u64 s_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) volatile u64 s_end_marker[] = LIMINE_REQUESTS_END_MARKER;

__attribute__((noreturn)) extern "C" void kmain()
{
    cpu::disable_interrupts();

    if (LIMINE_BASE_REVISION_SUPPORTED(s_base_revision) == false) {
        cpu::halt();
    }

    gdt::initialize();
    idt::initialize();

    cpu::enable_interrupts();

    cxxabi::construct();

    if (s_framebuffer_request.response == nullptr || s_framebuffer_request.response->framebuffer_count < 1) {
        cpu::halt();
    }

    const auto *framebuffer = s_framebuffer_request.response->framebuffers[0];
    volatile auto *framebuffer_ptr = static_cast<volatile u32 *>(framebuffer->address);
    for (usize y = 0; y < framebuffer->height; ++y) {
        for (usize x = 0; x < framebuffer->width; ++x) {
            const u32 green = (y * 255) / framebuffer->height;
            const u32 blue = (x * 255) / framebuffer->width;
            framebuffer_ptr[y * (framebuffer->pitch / 4) + x] = (green << 8) | blue;
        }
    }

    while (true) {
        cpu::disable_interrupts();
        cpu::halt();
    }
}

} // namespace kernel
