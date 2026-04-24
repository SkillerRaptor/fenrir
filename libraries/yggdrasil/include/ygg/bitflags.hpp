/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ygg/type_traits.hpp"

#define DECLARE_BITFLAG(Enum)                                                                       \
    inline constexpr Enum operator|(Enum lhs, Enum rhs)                                             \
    {                                                                                               \
        return static_cast<Enum>(::ygg::to_underlying(lhs) | ::ygg::to_underlying(rhs));            \
    }                                                                                               \
                                                                                                    \
    inline constexpr Enum operator&(Enum lhs, Enum rhs)                                             \
    {                                                                                               \
        return static_cast<Enum>(::ygg::to_underlying(lhs) & ::ygg::to_underlying(rhs));            \
    }                                                                                               \
                                                                                                    \
    inline constexpr Enum operator^(Enum lhs, Enum rhs)                                             \
    {                                                                                               \
        return static_cast<Enum>(::ygg::to_underlying(lhs) ^ ::ygg::to_underlying(rhs));            \
    }                                                                                               \
                                                                                                    \
    inline constexpr Enum operator~(Enum e) { return static_cast<Enum>(~::ygg::to_underlying(e)); } \
                                                                                                    \
    inline Enum &operator|=(Enum &lhs, Enum rhs)                                                    \
    {                                                                                               \
        return lhs = static_cast<Enum>(::ygg::to_underlying(lhs) | ::ygg::to_underlying(rhs));      \
    }                                                                                               \
                                                                                                    \
    inline Enum &operator&=(Enum &lhs, Enum rhs)                                                    \
    {                                                                                               \
        return lhs = static_cast<Enum>(::ygg::to_underlying(lhs) & ::ygg::to_underlying(rhs));      \
    }                                                                                               \
                                                                                                    \
    inline Enum &operator^=(Enum &lhs, Enum rhs)                                                    \
    {                                                                                               \
        return lhs = static_cast<Enum>(::ygg::to_underlying(lhs) ^ ::ygg::to_underlying(rhs));      \
    }
