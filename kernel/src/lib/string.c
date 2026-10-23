/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/string.h"

#include <stdint.h>

#ifdef memcpy
#    undef memcpy
#endif

void *memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    uint8_t *dst_ptr = dst;
    const uint8_t *src_ptr = src;

    for (size_t i = 0; i < n; ++i) {
        dst_ptr[i] = src_ptr[i];
    }

    return dst;
}

#ifdef memset
#    undef memset
#endif

void *memset(void *dst, int c, size_t n)
{
    uint8_t *dst_ptr = dst;
    for (size_t i = 0; i < n; ++i) {
        dst_ptr[i] = (uint8_t) c;
    }

    return dst;
}

#ifdef memmove
#    undef memmove
#endif

void *memmove(void *dst, const void *src, size_t n)
{
    uint8_t *dst_ptr = dst;
    const uint8_t *src_ptr = src;

    if ((uintptr_t) src > (uintptr_t) dst) {
        for (size_t i = 0; i < n; ++i) {
            dst_ptr[i] = src_ptr[i];
        }
    } else if ((uintptr_t) src < (uintptr_t) dst) {
        for (size_t i = n; i > 0; --i) {
            dst_ptr[i - 1] = src_ptr[i - 1];
        }
    }

    return dst;
}

#ifdef memcmp
#    undef memcmp
#endif

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *s1_ptr = s1;
    const uint8_t *s2_ptr = s2;

    for (size_t i = 0; i < n; ++i) {
        if (s1_ptr[i] != s2_ptr[i]) {
            return s1_ptr[i] < s2_ptr[i] ? -1 : 1;
        }
    }

    return 0;
}

#ifdef strlen
#    undef strlen
#endif

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }

    return len;
}
