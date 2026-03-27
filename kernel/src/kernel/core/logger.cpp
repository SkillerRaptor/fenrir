/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/core/logger.hpp"

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

#include "kernel/core/boot.hpp"

namespace kernel::logger {

static flanterm_context *s_context { nullptr };

void initialize()
{
    const auto *framebuffer = boot::get_framebuffer(0);

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

static void internal_write(const int c, void *)
{
    const auto character = static_cast<char>(c);
    if (character == '\n') {
        constexpr auto end_of_line = '\r';
        flanterm_write(s_context, &end_of_line, 1);
    }

    flanterm_write(s_context, &character, 1);
}

void log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    npf_vpprintf(internal_write, nullptr, format, args);
    va_end(args);
}

void ok(const char *format, ...)
{
    constexpr char level[] = "[   \033[38;2;0;128;0mOK\033[39m   ] ";
    flanterm_write(s_context, level, sizeof(level));

    va_list args;
    va_start(args, format);
    npf_vpprintf(internal_write, nullptr, format, args);
    va_end(args);
}

void info(const char *format, ...)
{
    constexpr char level[] = "[  \033[38;2;0;0;255mINFO\033[39m  ] ";
    flanterm_write(s_context, level, sizeof(level));

    va_list args;
    va_start(args, format);
    npf_vpprintf(internal_write, nullptr, format, args);
    va_end(args);
}

void warn(const char *format, ...)
{
    constexpr char level[] = "[  \033[38;2;255;215;0mWARN\033[39m  ] ";
    flanterm_write(s_context, level, sizeof(level));

    va_list args;
    va_start(args, format);
    npf_vpprintf(internal_write, nullptr, format, args);
    va_end(args);
}

void err(const char *format, ...)
{
    constexpr char level[] = "[ \033[38;2;255;0;0mFAILED\033[39m ] ";
    flanterm_write(s_context, level, sizeof(level));

    va_list args;
    va_start(args, format);
    npf_vpprintf(internal_write, nullptr, format, args);
    va_end(args);
}

} // namespace kernel::logger
