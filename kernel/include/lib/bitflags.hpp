/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/type_traits.hpp"

#define DECLARE_BITFLAG(Enum)                                                                     \
    inline constexpr Enum operator|(Enum lhs, Enum rhs)                                           \
    {                                                                                             \
        return static_cast<Enum>(lib::to_underlying(lhs) | lib::to_underlying(rhs));              \
    }                                                                                             \
                                                                                                  \
    inline constexpr Enum operator&(Enum lhs, Enum rhs)                                           \
    {                                                                                             \
        return static_cast<Enum>(lib::to_underlying(lhs) & lib::to_underlying(rhs));              \
    }                                                                                             \
                                                                                                  \
    inline constexpr Enum operator^(Enum lhs, Enum rhs)                                           \
    {                                                                                             \
        return static_cast<Enum>(lib::to_underlying(lhs) ^ lib::to_underlying(rhs));              \
    }                                                                                             \
                                                                                                  \
    inline constexpr Enum operator~(Enum e) { return static_cast<Enum>(~lib::to_underlying(e)); } \
                                                                                                  \
    inline Enum &operator|=(Enum &lhs, Enum rhs)                                                  \
    {                                                                                             \
        return lhs = static_cast<Enum>(lib::to_underlying(lhs) | lib::to_underlying(rhs));        \
    }                                                                                             \
                                                                                                  \
    inline Enum &operator&=(Enum &lhs, Enum rhs)                                                  \
    {                                                                                             \
        return lhs = static_cast<Enum>(lib::to_underlying(lhs) & lib::to_underlying(rhs));        \
    }                                                                                             \
                                                                                                  \
    inline Enum &operator^=(Enum &lhs, Enum rhs)                                                  \
    {                                                                                             \
        return lhs = static_cast<Enum>(lib::to_underlying(lhs) ^ lib::to_underlying(rhs));        \
    }
