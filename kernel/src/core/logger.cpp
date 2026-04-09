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
#include <nanoprintf.h>
#include <stdarg.h>

#include "core/boot.hpp"
#include "drivers/serial.hpp"
#include "lib/string.hpp"
#include "sync/spinlock.hpp"

namespace logger {

static flanterm_context *s_context { nullptr };
static Spinlock s_lock { };

void initialize()
{
    const limine_framebuffer *framebuffer = boot::get_framebuffer(0);

    s_context = flanterm_fb_init(
        nullptr,
        nullptr,
        static_cast<uint32_t *>(framebuffer->address),
        framebuffer->width,
        framebuffer->height,
        framebuffer->pitch,
        framebuffer->red_mask_size,
        framebuffer->red_mask_shift,
        framebuffer->green_mask_size,
        framebuffer->green_mask_shift,
        framebuffer->blue_mask_size,
        framebuffer->blue_mask_shift,
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

static void write_character(const int c, void *)
{
    const char character = static_cast<char>(c);
    if (character == '\n') {
        write_character('\r', nullptr);
    }

    s_lock.lock();
    flanterm_write(s_context, &character, 1);
    s_lock.unlock();

    serial::write(character);
}

static void write_string(const char *str)
{
    for (usize i = 0; i < strlen(str); i++) {
        write_character(str[i], nullptr);
    }
}

void log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    npf_vpprintf(write_character, nullptr, format, args);
    va_end(args);

    write_string("\033[0m");
}

void info(const char *format, ...)
{
    write_string("\033[38;2;0;128;0minfo\033[39m: ");

    va_list args;
    va_start(args, format);
    npf_vpprintf(write_character, nullptr, format, args);
    va_end(args);

    write_string("\033[0m");
}

void debug(const char *format, ...)
{
    write_string("\033[38;2;0;0;255mdebug\033[39m: ");

    va_list args;
    va_start(args, format);
    npf_vpprintf(write_character, nullptr, format, args);
    va_end(args);

    write_string("\033[0m");
}

void warn(const char *format, ...)
{
    write_string("\033[38;2;255;215;0mwarn\033[39m: ");

    va_list args;
    va_start(args, format);
    npf_vpprintf(write_character, nullptr, format, args);
    va_end(args);
    write_string("\033[0m");
}

void err(const char *format, ...)
{
    write_string("\033[38;2;255;0;0merror\033[39m: ");

    va_list args;
    va_start(args, format);
    npf_vpprintf(write_character, nullptr, format, args);
    va_end(args);

    write_string("\033[0m");
}

} // namespace logger
