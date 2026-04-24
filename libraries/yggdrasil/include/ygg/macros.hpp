/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#define TRY(expression)                                      \
    ({                                                       \
        auto _result = (expression);                         \
        if (!(_result)) {                                    \
            return _result.unwrap_other();                   \
        }                                                    \
        static_cast<decltype(_result) &&>(_result).unwrap(); \
    })
