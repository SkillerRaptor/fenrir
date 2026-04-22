/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/format.hpp"
#include "lib/string_view.hpp"

namespace logger {

enum class Level {
    None = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

void initialize();

namespace detail {

void lock();
void unlock();

void write_character(char);
void write_string(StringView);

void write_timestamp();

} // namespace detail

template <typename... Args>
void log(const Level level, const StringView fmt, Args &&...args)
{
    detail::lock();

    switch (level) {
    case Level::Debug:
        detail::write_timestamp();
        detail::write_string("\033[38;2;0;0;255mdebug\033[39m: ");
        break;
    case Level::Info:
        detail::write_timestamp();
        detail::write_string(" \033[38;2;0;128;0minfo\033[39m: ");
        break;
    case Level::Warning:
        detail::write_timestamp();
        detail::write_string(" \033[38;2;255;215;0mwarn\033[39m: ");
        break;
    case Level::Error:
        detail::write_timestamp();
        detail::write_string("\033[38;2;255;0;0merror\033[39m: ");
        break;
    case Level::Fatal:
        detail::write_string("\033[38;2;128;0;0mfatal\033[39m: ");
        break;
    case Level::None:
    default:
        break;
    }

    fmt::format(detail::write_character, fmt, args...);

    detail::unlock();
}

template <typename... Args>
void debug(const StringView fmt, Args &&...args)
{
    log(Level::Debug, fmt, args...);
}

template <typename... Args>
void info(const StringView fmt, Args &&...args)
{
    log(Level::Info, fmt, args...);
}

template <typename... Args>
void warn(const StringView fmt, Args &&...args)
{
    log(Level::Warning, fmt, args...);
}

template <typename... Args>
void error(const StringView fmt, Args &&...args)
{
    log(Level::Error, fmt, args...);
}

template <typename... Args>
void fatal(const StringView fmt, Args &&...args)
{
    log(Level::Fatal, fmt, args...);
}

template <typename... Args>
void print(const StringView fmt, Args &&...args)
{
    log(Level::None, fmt, args...);
}

} // namespace logger
