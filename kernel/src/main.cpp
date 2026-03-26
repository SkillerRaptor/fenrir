/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((used, section(".limine_requests"))) volatile uint64_t s_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests"))) volatile limine_framebuffer_request s_framebuffer_request
    = { .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0, .response = nullptr };

__attribute__((used, section(".limine_requests_start"))) volatile uint64_t s_start_marker[]
    = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) volatile uint64_t s_end_marker[] = LIMINE_REQUESTS_END_MARKER;

extern "C" {

int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
void __cxa_pure_virtual() { asm volatile("hlt"); }
void *__dso_handle;
}

extern void (*__init_array[])();
extern void (*__init_array_end[])();

__attribute__((noreturn)) extern "C" void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED(s_base_revision) == false) {
        asm volatile("hlt");
    }

    for (size_t i = 0; &__init_array[i] != __init_array_end; ++i) {
        __init_array[i]();
    }

    if (s_framebuffer_request.response == nullptr || s_framebuffer_request.response->framebuffer_count < 1) {
        asm volatile("hlt");
    }

    limine_framebuffer *framebuffer = s_framebuffer_request.response->framebuffers[0];

    volatile auto *framebuffer_ptr = static_cast<volatile uint32_t *>(framebuffer->address);
    for (size_t y = 0; y < framebuffer->height; ++y) {
        for (size_t x = 0; x < framebuffer->width; ++x) {
            uint32_t green = (y * 255) / framebuffer->height;
            uint32_t blue = (x * 255) / framebuffer->width;
            framebuffer_ptr[y * (framebuffer->pitch / 4) + x] = (green << 8) | blue;
        }
    }

    while (true) {
        asm volatile("hlt");
    }
}
