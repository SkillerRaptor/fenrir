/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace logger {

void initialize();

void log(const char *format, ...) __attribute__((format(printf, 1, 2)));
void info(const char *format, ...) __attribute__((format(printf, 1, 2)));
void debug(const char *format, ...) __attribute__((format(printf, 1, 2)));
void warn(const char *format, ...) __attribute__((format(printf, 1, 2)));
void err(const char *format, ...) __attribute__((format(printf, 1, 2)));
void fatal(const char *format, ...) __attribute__((format(printf, 1, 2)));

} // namespace logger
