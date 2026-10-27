/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

void pmm_init();

void *pmm_alloc(size_t page_count);
void *pmm_calloc(size_t page_count);
void pmm_free(void *ptr, size_t page_count);
