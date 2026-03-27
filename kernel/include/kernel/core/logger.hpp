/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace kernel::logger {

void initialize();

void log(const char *format, ...);
void ok(const char *format, ...);
void info(const char *format, ...);
void warn(const char *format, ...);
void err(const char *format, ...);

} // namespace kernel::logger
