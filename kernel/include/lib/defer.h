/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#if __has_include(<stddefer.h>)
#    include <stddefer.h>
#else
#    error "Defer is not supported by the compiler"
#endif
