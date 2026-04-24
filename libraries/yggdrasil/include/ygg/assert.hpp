/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/string_view.hpp"
#include "ygg/types.hpp"

#ifdef NDEBUG
#    define ASSERT(assertion) ((void) 0)
#else
#    define ASSERT(assertion)                                                                           \
        do {                                                                                            \
            if (!(assertion)) {                                                                         \
                ::ygg::detail::__assertion_failed(#assertion, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
            }                                                                                           \
        } while (0)
#endif

namespace ygg::detail {

[[noreturn]] void __assertion_failed(StringView assertion, StringView file, u32 line, StringView function);

} // namespace ygg::detail
