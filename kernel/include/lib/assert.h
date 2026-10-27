/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#ifdef NDEBUG
#    define ASSERT(assertion) ((void) 0)
#else
#    define ASSERT(assertion)                                                 \
        do {                                                                  \
            if (!(assertion)) {                                               \
                __assertion_failed(#assertion, __FILE__, __LINE__, __func__); \
            }                                                                 \
        } while (0)
#endif

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, uint32_t line, const char *function);
