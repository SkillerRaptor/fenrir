/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lib/span.hpp"
#include "lib/string_view.hpp"

namespace fmt {

namespace detail {

struct Argument {
    enum class Kind : u8 {
        None = 0,
        I64,
        U64,
        Pointer,
        String,
        Character,
        Boolean,
    };

    Kind kind = Kind::None;

    union {
        i64 as_i64;
        u64 as_u64;
        const void *as_pointer;
        const char *as_string;
        char as_character;
        bool as_boolean;
    };
};

template <typename T>
Argument create_argument(T) = delete;

template <>
Argument create_argument(i8);

template <>
Argument create_argument(i16);

template <>
Argument create_argument(i32);

template <>
Argument create_argument(i64);

template <>
Argument create_argument(u8);

template <>
Argument create_argument(u16);

template <>
Argument create_argument(u32);

template <>
Argument create_argument(u64);

template <typename T>
Argument create_argument(T *value)
{
    return {
        .kind = Argument::Kind::Pointer,
        .as_pointer = static_cast<const void *>(value),
    };
}

template <typename T>
Argument create_argument(const T *value)
{
    return {
        .kind = Argument::Kind::Pointer,
        .as_pointer = static_cast<const void *>(value),
    };
}

template <>
Argument create_argument(void *);

template <>
Argument create_argument(const void *);

template <>
Argument create_argument(const char *);

template <>
Argument create_argument(char);

template <>
Argument create_argument(bool);

void populate_arguments(Argument *, usize);

template <typename T, typename... Args>
void populate_arguments(Argument *arguments, const usize index, T &&first, Args &&...args)
{
    arguments[index] = create_argument(first);
    populate_arguments(arguments, index + 1, args...);
}

void do_format(void (*write_character)(char), StringView fmt, Span<Argument>);

} // namespace detail

inline void format(void (*write_character)(char), const StringView fmt)
{
    for (const char c : fmt) {
        write_character(c);
    }
}

template <typename... Args>
void format(void (*write_character)(char), const StringView fmt, Args &&...args)
{
    static constexpr usize n = sizeof...(Args);

    detail::Argument arguments[n] { };
    detail::populate_arguments(arguments, 0, args...);

    detail::do_format(write_character, fmt, arguments);
}

} // namespace fmt
