/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/console.h"

#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include "boot/boot.h"
#include "lib/string.h"

static struct flanterm_context *s_context = nullptr;

void console_init(void)
{
    const struct limine_framebuffer *framebuffer = boot_get_framebuffer_response()->framebuffers[0];

    s_context = flanterm_fb_init(
        nullptr,
        nullptr,
        framebuffer->address,
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
        0,
        true);
}

void console_putc(int c)
{
    char ch = (char) c;
    if (ch == '\n') {
        flanterm_write(s_context, "\r", 1);
    }

    flanterm_write(s_context, &ch, 1);
}

void console_puts(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++) {
        console_putc(str[i]);
    }
}
