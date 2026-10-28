/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#ifdef NDEBUG
#    define ASSERT(assertion, ...) ((void) 0)
#else
#    define ASSERT(assertion, ...)                                                           \
        do {                                                                                 \
            if (!(assertion)) {                                                              \
                _assertion_failed(#assertion, __FILE__, __LINE__, __func__, "" __VA_ARGS__); \
            }                                                                                \
        } while (false)
#endif

#define VERIFY(assertion, ...)                                                           \
    do {                                                                                 \
        if (!(assertion)) {                                                              \
            _assertion_failed(#assertion, __FILE__, __LINE__, __func__, "" __VA_ARGS__); \
        }                                                                                \
    } while (false)

#define PANIC(message, ...)                                                       \
    do {                                                                          \
        _panic(__FILE__, __LINE__, __func__, message __VA_OPT__(, ) __VA_ARGS__); \
    } while (false)

[[noreturn]] void _assertion_failed(
    const char *assertion, const char *file, uint32_t line, const char *function, const char *message);
[[noreturn]] void _panic(const char *file, uint32_t line, const char *function, const char *fmt, ...);
