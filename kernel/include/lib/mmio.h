/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

static inline uint8_t mmio_read8(void *ptr) { return *((volatile uint8_t *) ptr); }
static inline uint16_t mmio_read16(void *ptr) { return *((volatile uint16_t *) ptr); }
static inline uint32_t mmio_read32(void *ptr) { return *((volatile uint32_t *) ptr); }
static inline uint64_t mmio_read64(void *ptr) { return *((volatile uint64_t *) ptr); }

static inline void mmio_write8(void *ptr, uint8_t value) { *((volatile uint8_t *) ptr) = value; }
static inline void mmio_write16(void *ptr, uint16_t value) { *((volatile uint16_t *) ptr) = value; }
static inline void mmio_write32(void *ptr, uint32_t value) { *((volatile uint32_t *) ptr) = value; }
static inline void mmio_write64(void *ptr, uint64_t value) { *((volatile uint64_t *) ptr) = value; }
