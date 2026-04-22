/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/format.hpp"

namespace fmt::detail {

template <>
Argument create_argument(const i8 value)
{
    return {
        .kind = Argument::Kind::I64,
        .as_i64 = value,
    };
}

template <>
Argument create_argument(const i16 value)
{
    return {
        .kind = Argument::Kind::I64,
        .as_i64 = value,
    };
}

template <>
Argument create_argument(const i32 value)
{
    return {
        .kind = Argument::Kind::I64,
        .as_i64 = value,
    };
}

template <>
Argument create_argument(const i64 value)
{
    return {
        .kind = Argument::Kind::I64,
        .as_i64 = value,
    };
}

template <>
Argument create_argument(const u8 value)
{
    return {
        .kind = Argument::Kind::U64,
        .as_u64 = value,
    };
}

template <>
Argument create_argument(const u16 value)
{
    return {
        .kind = Argument::Kind::U64,
        .as_u64 = value,
    };
}

template <>
Argument create_argument(const u32 value)
{
    return {
        .kind = Argument::Kind::U64,
        .as_u64 = value,
    };
}

template <>
Argument create_argument(const u64 value)
{
    return {
        .kind = Argument::Kind::U64,
        .as_u64 = value,
    };
}

template <>
Argument create_argument(void *value)
{
    return {
        .kind = Argument::Kind::Pointer,
        .as_pointer = value,
    };
}

template <>
Argument create_argument(const void *value)
{
    return {
        .kind = Argument::Kind::Pointer,
        .as_pointer = value,
    };
}

template <>
Argument create_argument(char *value)
{
    return {
        .kind = Argument::Kind::String,
        .as_string = value,
    };
}

template <>
Argument create_argument(const char *value)
{
    return {
        .kind = Argument::Kind::String,
        .as_string = value,
    };
}

template <>
Argument create_argument(const char value)
{
    return {
        .kind = Argument::Kind::Character,
        .as_character = value,
    };
}

template <>
Argument create_argument(const bool value)
{
    return {
        .kind = Argument::Kind::Boolean,
        .as_boolean = value,
    };
}

void populate_arguments(Argument *, usize) { }

struct Specification {
    u8 base = 10;
    bool upper = false;
    bool prefix = false;
    bool is_character = false;
    bool is_string = false;
    bool is_pointer = false;
    usize width = 0;
    bool zero_padding = false;
};

static Specification parse_spec(const StringView::Iterator start, const StringView::Iterator end)
{
    StringView::Iterator it = start;

    Specification specification { };
    if (it == end) {
        return specification;
    }

    if (*it == ':') {
        ++it;
    }

    if (it == end) {
        return specification;
    }

    if (*it == '#') {
        specification.prefix = true;
        ++it;
    }

    if (it == end) {
        return specification;
    }

    if (*it == '0') {
        specification.zero_padding = true;
        ++it;
    }

    while (it != end && *it >= '0' && *it <= '9') {
        specification.width = specification.width * 10 + static_cast<usize>(*it - '0');
        ++it;
    }

    if (it == end) {
        return specification;
    }

    switch (*it) {
    case 'd':
    case 'u':
        specification.base = 10;
        break;
    case 'x':
        specification.base = 16;
        specification.upper = false;
        break;
    case 'X':
        specification.base = 16;
        specification.upper = true;
        break;
    case 'o':
        specification.base = 8;
        break;
    case 'b':
        specification.base = 2;
        break;
    case 'c':
        specification.is_character = true;
        break;
    case 's':
        specification.is_string = true;
        break;
    case 'p':
        specification.is_pointer = true;
        specification.base = 16;
        specification.prefix = true;
        break;
    default:
        break;
    }

    return specification;
}

static void write_string(void (*write_character)(char), const char *str)
{
    if (!str) {
        str = "<null>";
    }

    while (*str) {
        write_character(*str++);
    }
}

static void write_uint(void (*write_character)(char), const u64 value, const Specification &spec)
{
    const char *digits = spec.upper ? "0123456789ABCDEF" : "0123456789abcdef";

    char buffer[64] { };
    usize length = 0;

    u64 integer_value = value;
    if (integer_value == 0) {
        buffer[length++] = '0';
    } else {
        while (integer_value > 0) {
            buffer[length++] = digits[integer_value % spec.base];
            integer_value /= spec.base;
        }
    }

    const char *prefix = nullptr;
    if (spec.prefix) {
        if (spec.base == 16) {
            prefix = spec.upper ? "0X" : "0x";
        } else if (spec.base == 2) {
            prefix = "0b";
        } else if (spec.base == 8) {
            prefix = "0o";
        }
    }

    const usize prefix_length = prefix ? 2 : 0;

    const usize total = prefix_length + length;
    const usize padding = spec.width > total ? spec.width - total : 0;
    const char padding_character = spec.zero_padding ? '0' : ' ';

    if (spec.zero_padding && prefix) {
        write_string(write_character, prefix);
        for (usize i = 0; i < padding; ++i) {
            write_character('0');
        }
    } else {
        for (usize i = 0; i < padding; ++i) {
            write_character(padding_character);
        }

        if (prefix) {
            write_string(write_character, prefix);
        }
    }

    for (usize i = length; i > 0; --i) {
        write_character(buffer[i - 1]);
    }
}

static void write_int(void (*write_character)(char), const i64 value, const Specification &spec)
{
    if (value < 0 && spec.base == 10) {
        write_character('-');
        write_uint(write_character, static_cast<u64>(-value), spec);
    } else {
        write_uint(write_character, static_cast<u64>(value), spec);
    }
}

static void format_argument(void (*write_character)(char), const Argument &argument, const Specification &spec)
{
    switch (argument.kind) {
    case Argument::Kind::I64:
        write_int(write_character, argument.as_i64, spec);
        break;
    case Argument::Kind::U64:
        write_uint(write_character, argument.as_u64, spec);
        break;
    case Argument::Kind::Pointer: {
        Specification ptr_spec = spec;
        ptr_spec.base = 16;
        ptr_spec.prefix = true;
        ptr_spec.zero_padding = true;
        ptr_spec.width = 18;
        write_uint(write_character, reinterpret_cast<u64>(argument.as_pointer), ptr_spec);
        break;
    }
    case Argument::Kind::String:
        write_string(write_character, argument.as_string);
        break;
    case Argument::Kind::Character:
        if (spec.is_character || spec.base == 10) {
            write_character(argument.as_character);
        } else {
            write_uint(write_character, static_cast<u64>(argument.as_character), spec);
        }
        break;
    case Argument::Kind::Boolean:
        if (spec.base == 10 && !spec.is_string) {
            write_uint(write_character, argument.as_boolean ? 1 : 0, spec);
        } else {
            write_string(write_character, argument.as_boolean ? "true" : "false");
        }
        break;
    case Argument::Kind::None:
    default:
        __builtin_unreachable();
    }
}

void do_format(void (*write_character)(char), const StringView fmt, const Span<Argument> arguments)
{
    usize argument_index = 0;
    StringView::Iterator it = fmt.begin();
    while (it != fmt.end()) {
        if (*it == '}') {
            StringView::Iterator next = it + 1;
            if (next != fmt.end() && *next == '}') {
                write_character('}');
                it = next + 1;
                continue;
            }

            write_character(*it);
            ++it;
            continue;
        }

        if (*it != '{') {
            write_character(*it);
            ++it;
            continue;
        }

        StringView::Iterator next = it + 1;
        if (next != fmt.end() && *next == '{') {
            write_character('{');
            it = next + 1;
            continue;
        }

        const StringView::Iterator spec_start = next;
        StringView::Iterator spec_end = spec_start;
        while (spec_end != fmt.end() && *spec_end != '}') {
            ++spec_end;
        }

        if (spec_end == fmt.end()) {
            while (it != fmt.end()) {
                write_character(*it);
                ++it;
            }

            break;
        }

        const Specification spec = parse_spec(spec_start, spec_end);
        if (argument_index < arguments.size()) {
            format_argument(write_character, arguments[argument_index], spec);
            ++argument_index;
        } else {
            write_character('{');
            write_character('?');
            write_character('}');
        }

        it = spec_end + 1;
    }
}

} // namespace fmt::detail
