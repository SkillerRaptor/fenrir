/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "klibc/string.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *str)
{
    const char *end = str;

    while (*end != '\0') {
        ++end;
    }

    return end - str;
}

int memcmp(const void *lhs, const void *rhs, const size_t n)
{
    const uint8_t *lhs_ptr = static_cast<const uint8_t *>(lhs);
    const uint8_t *rhs_ptr = static_cast<const uint8_t *>(rhs);

    for (size_t i = 0; i < n; i++) {
        const uint8_t lhs_value = lhs_ptr[i];
        const uint8_t rhs_value = rhs_ptr[i];
        if (lhs_value != rhs_value) {
            return lhs_value < rhs_value ? -1 : 1;
        }
    }

    return 0;
}

void *memset(void *dst, const int c, const size_t n)
{
    uint8_t *dst_ptr = static_cast<uint8_t *>(dst);

    for (size_t i = 0; i < n; i++) {
        dst_ptr[i] = static_cast<uint8_t>(c);
    }

    return dst;
}

void *memcpy(void *__restrict dst, const void *__restrict src, const size_t n)
{
    uint8_t *dst_ptr = static_cast<uint8_t *>(dst);
    const uint8_t *src_ptr = static_cast<const uint8_t *>(src);

    for (size_t i = 0; i < n; i++) {
        dst_ptr[i] = src_ptr[i];
    }

    return dst;
}

void *memmove(void *dst, const void *src, const size_t n)
{
    uint8_t *dst_ptr = static_cast<uint8_t *>(dst);
    const uint8_t *src_ptr = static_cast<const uint8_t *>(src);

    if (src > dst) {
        for (size_t i = 0; i < n; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    } else if (src < dst) {
        for (size_t i = n; i > 0; i--) {
            dst_ptr[i - 1] = src_ptr[i - 1];
        }
    }

    return dst;
}

#ifdef __cplusplus
}
#endif
