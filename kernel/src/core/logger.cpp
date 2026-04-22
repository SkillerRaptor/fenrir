/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "core/logger.hpp"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_IMPLEMENTATION

#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <stdarg.h>

#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "drivers/serial.hpp"
#include "lib/string.hpp"
#include "sync/spinlock.hpp"

namespace logger {

static flanterm_context *s_context = nullptr;
static Spinlock s_lock { };

static u64 s_tsc_frequency = 0;
static u64 s_tsc_boot = 0;

struct Timestamp {
    u64 seconds = 0;
    u64 milliseconds = 0;
};

static u64 get_tsc()
{
    u32 low = 0;
    u32 high = 0;
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    return (static_cast<u64>(high) << 32) | low;
}

static Timestamp get_timestamp()
{
    const u64 elapsed = get_tsc() - s_tsc_boot;
    const u64 seconds = elapsed / s_tsc_frequency;
    const u64 remainder = elapsed % s_tsc_frequency;
    const u64 milliseconds = (remainder * 1'000ull) / s_tsc_frequency;
    return { seconds, milliseconds };
}

void initialize()
{
    s_tsc_frequency = boot::get_tsc_frequency();
    s_tsc_boot = get_tsc();

    const Span<limine_framebuffer *> framebuffers = boot::get_framebuffers();

    s_context = flanterm_fb_init(
        nullptr,
        nullptr,
        static_cast<uint32_t *>(framebuffers[0]->address),
        framebuffers[0]->width,
        framebuffers[0]->height,
        framebuffers[0]->pitch,
        framebuffers[0]->red_mask_size,
        framebuffers[0]->red_mask_shift,
        framebuffers[0]->green_mask_size,
        framebuffers[0]->green_mask_shift,
        framebuffers[0]->blue_mask_size,
        framebuffers[0]->blue_mask_shift,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0,
        0,
        1,
        0,
        0,
        0,
        FLANTERM_FB_ROTATE_0);
}

namespace detail {

void lock()
{
    cpu::enter_critical();
    s_lock.lock();
}

void unlock()
{
    s_lock.unlock();
    cpu::leave_critical();
}

void write_character(const char c)
{
    if (c == '\n') {
        write_character('\r');
    }

    flanterm_write(s_context, &c, 1);
    serial::write(c);
}

void write_string(const StringView string)
{
    for (const char c : string) {
        write_character(c);
    }
}

void write_timestamp()
{
    const Timestamp ts = get_timestamp();
    fmt::format(write_character, "[{}.{:03u}] ", ts.seconds, ts.milliseconds);
}

} // namespace detail

} // namespace logger
