/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdarg.h>

enum log_level {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

[[gnu::format(printf, 2, 3)]] void log_log(enum log_level level, const char *fmt, ...);

void log_vlog(enum log_level level, const char *fmt, va_list args);

#define LOG_ERROR(fmt, ...) log_log(LOG_LEVEL_ERROR, fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(fmt, ...) log_log(LOG_LEVEL_WARNING, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_INFO(fmt, ...) log_log(LOG_LEVEL_INFO, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_DEBUG(fmt, ...) log_log(LOG_LEVEL_DEBUG, fmt __VA_OPT__(, ) __VA_ARGS__);
#define LOG_TRACE(fmt, ...) log_log(LOG_LEVEL_TRACE, fmt __VA_OPT__(, ) __VA_ARGS__);
