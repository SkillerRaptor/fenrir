/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/types.hpp"

template <typename Key>
struct Hash {
    usize operator()(const Key &key) const = delete;
};

template <>
struct Hash<u64> {
    usize operator()(const u64 &key) const
    {
        u64 result = 0xcbf29ce484222325;
        for (usize i { 0 }; i < 4; ++i) {
            const u8 byte = (key >> (i * 8)) & 0xff;
            result = (result ^ byte) * 0x00000100000001b3;
        }
        return result;
    }
};

template <>
struct Hash<u32> {
    usize operator()(const u32 &key) const { return Hash<u64> { }(key); }
};
