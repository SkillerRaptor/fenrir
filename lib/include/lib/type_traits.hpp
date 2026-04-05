/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace lib {

template <class T>
using UnderlyingType = __underlying_type(T);

template <class Enum>
constexpr UnderlyingType<Enum> to_underlying(const Enum e) noexcept
{
    return static_cast<UnderlyingType<Enum>>(e);
}

} // namespace lib
