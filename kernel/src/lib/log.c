/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/log.h"

#include <stddef.h>

#include "lib/stdio.h"

void log_log(enum log_level level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_vlog(level, fmt, args);
    va_end(args);
}

void log_vlog(enum log_level level, const char *fmt, va_list args)
{
    char *level_color = "";
    char *level_string = "<unknown>";
    switch (level) {
    case LOG_LEVEL_ERROR:
        level_color = "\x1b[38;2;255;0;0m";
        level_string = "error";
        break;
    case LOG_LEVEL_WARNING:
        level_color = "\x1b[38;2;255;215;0m";
        level_string = "warn";
        break;
    case LOG_LEVEL_INFO:
        level_color = "\x1b[38;2;0;128;0m";
        level_string = "info";
        break;
    case LOG_LEVEL_DEBUG:
        level_color = "\x1b[38;2;50;50;255m";
        level_string = "debug";
        break;
    case LOG_LEVEL_TRACE:
        level_color = "\x1b[38;2;170;68;255m";
        level_string = "trace";
        break;
    }

    printf("%s%s\x1b[0m: ", level_color, level_string);
    vprintf(fmt, args);
}
