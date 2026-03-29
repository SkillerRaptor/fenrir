/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

using i8 = __INT8_TYPE__;
static_assert(sizeof(i8) == 1);

using i16 = __INT16_TYPE__;
static_assert(sizeof(i16) == 2);

using i32 = __INT32_TYPE__;
static_assert(sizeof(i32) == 4);

using i64 = __INT64_TYPE__;
static_assert(sizeof(i64) == 8);

using u8 = __UINT8_TYPE__;
static_assert(sizeof(u8) == 1);

using u16 = __UINT16_TYPE__;
static_assert(sizeof(u16) == 2);

using u32 = __UINT32_TYPE__;
static_assert(sizeof(u32) == 4);

using u64 = __UINT64_TYPE__;
static_assert(sizeof(u64) == 8);

using isize = __PTRDIFF_TYPE__;
static_assert(sizeof(isize) == 8);

using usize = __SIZE_TYPE__;
static_assert(sizeof(usize) == 8);
