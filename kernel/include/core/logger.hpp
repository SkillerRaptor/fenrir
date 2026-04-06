/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace logger {

void initialize();

void log(const char *format, ...);
void info(const char *format, ...);
void debug(const char *format, ...);
void warn(const char *format, ...);
void err(const char *format, ...);

} // namespace logger
