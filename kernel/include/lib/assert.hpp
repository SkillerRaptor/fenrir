/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#    define assert(assertion) ((void) 0)
#else
#    define assert(assertion)                                                 \
        do {                                                                  \
            if (!(assertion)) {                                               \
                __assertion_failed(#assertion, __FILE__, __LINE__, __func__); \
            }                                                                 \
        } while (0)
#endif

[[noreturn]] void __assertion_failed(const char *assertion, const char *file, u32 line, const char *function);

#ifdef __cplusplus
}
#endif
