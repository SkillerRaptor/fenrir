/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "filesystem/ustar.hpp"

#include <ygg/string.hpp>

namespace ustar {

usize octal_to_binary(const u8 *array, usize size)
{
    usize count = 0;

    const u8 *character = array;
    while (size-- > 0) {
        count *= 8;
        count += *character - '0';
        ++character;
    }

    return count;
}

File lookup(const u8 *archive, const char *file_name)
{
    const u8 *ptr = archive;
    while (!memcmp(ptr + 257, "ustar", 5)) {
        const usize file_size = octal_to_binary(ptr + 0x7c, 11);
        if (!memcmp(ptr, file_name, strlen(file_name) + 1)) {
            return {
                .ptr = ptr + 512,
                .size = file_size,
            };
        }
        ptr += (((file_size + 511) / 512) + 1) * 512;
    }

    return {
        .ptr = nullptr,
        .size = 0,
    };
}

} // namespace ustar
