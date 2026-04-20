/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "core/stacktrace.hpp"

#include "core/boot.hpp"
#include "core/logger.hpp"
#include "lib/assert.hpp"
#include "lib/string.hpp"

namespace stacktrace {

struct StackFrame {
    StackFrame *rbp = nullptr;
    u64 rip = 0;
};

struct Symbol {
    const char *name = nullptr;
    u64 address = 0;
};

static char *s_file_buffer = nullptr;
static Symbol *s_symbols = nullptr;
static usize s_symbol_count = 0;

static bool is_newline(const char c) { return c == '\n' || c == '\r'; }

static usize parse_hex(const char *src, u64 &out)
{
    out = 0;
    usize i = 0;

    for (; i < 16 && src[i]; ++i) {
        const char c = src[i];
        u8 nibble;
        if (c >= '0' && c <= '9') {
            nibble = static_cast<u8>(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            nibble = static_cast<u8>(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            nibble = static_cast<u8>(c - 'A' + 10);
        } else {
            break;
        }
        out = (out << 4) | nibble;
    }

    return i;
}

void initialize()
{
    const limine_file *symbol_map_file = boot::get_modules()[0];

    logger::debug(
        "Stacktrace: Kernel symbol map found at 0x%016lx with %zu bytes\n",
        reinterpret_cast<u64>(symbol_map_file->address),
        symbol_map_file->size);

    s_file_buffer = new char[symbol_map_file->size + 1];
    memcpy(s_file_buffer, symbol_map_file->address, symbol_map_file->size);
    s_file_buffer[symbol_map_file->size] = '\0';

    usize count = 0;
    for (usize i = 0; i < symbol_map_file->size; ++i) {
        if (is_newline(s_file_buffer[i])) {
            ++count;
        }
    }

    if (symbol_map_file->size > 0 && !is_newline(s_file_buffer[symbol_map_file->size - 1])) {
        ++count;
    }

    s_symbols = new Symbol[count];

    char *current = s_file_buffer;
    const char *end = s_file_buffer + symbol_map_file->size;
    while (current < end) {
        const char *line_start = current;
        while (current < end && !is_newline(*current)) {
            ++current;
        }

        char *line_end = current;
        while (current < end && is_newline(*current)) {
            ++current;
        }

        if (line_end - line_start < 19) {
            continue;
        }

        u64 addr = 0;
        const usize parsed = parse_hex(line_start, addr);
        if (parsed != 16) {
            continue;
        }

        if (line_start[16] != ' ') {
            continue;
        }

        if (line_start[18] != ' ') {
            continue;
        }

        const char *name_start = line_start + 19;
        if (name_start >= line_end) {
            continue;
        }

        *line_end = '\0';

        s_symbols[s_symbol_count].address = addr;
        s_symbols[s_symbol_count].name = name_start;
        ++s_symbol_count;
    }

    logger::info("Stacktrace: Initialized\n");
}

static const Symbol *find_symbol(const u64 rip)
{
    if (s_symbol_count == 0) {
        return nullptr;
    }

    usize low = 0;
    usize high = s_symbol_count;
    while (low + 1 < high) {
        const usize mid = low + (high - low) / 2;
        if (s_symbols[mid].address <= rip) {
            low = mid;
        } else {
            high = mid;
        }
    }

    if (s_symbols[low].address > rip) {
        return nullptr;
    }

    return &s_symbols[low];
}

void print(const u64 max_frames, const u64 rip_argument)
{
    assert(max_frames > 0);

    volatile StackFrame *stack_frame = nullptr;
    asm volatile("mov %%rbp, %0" : "=r"(stack_frame) : : "memory");

    logger::fatal("Stacktrace:\n");

    for (usize i = 0; stack_frame && i < max_frames; ++i) {
        const uint64_t rip = stack_frame->rip == 0x1 ? rip_argument : stack_frame->rip;
        if (rip == 0) {
            break;
        }

        // NOTE: Check for misaligned address
        const u64 address = reinterpret_cast<u64>(stack_frame->rbp);
        if (address != 0 && (address & 0x7)) {
            break;
        }

        stack_frame = stack_frame->rbp;

        const Symbol *symbol = find_symbol(rip);
        if (!symbol) {
            logger::fatal("  %02lu. \033[38;2;0;0;255m0x%016lx \033[0min \033[38;2;255;215;0m??\n", i + 1, rip);
            continue;
        }

        const u64 offset = rip - symbol->address;
        if (offset == 0) {
            logger::fatal(
                "  %02lu. \033[38;2;0;0;255m0x%016lx \033[0min \033[38;2;255;215;0m%s\n",
                i + 1,
                rip,
                symbol->name);
        } else {
            logger::fatal(
                "  %02lu. \033[38;2;0;0;255m0x%016lx \033[0min \033[38;2;255;215;0m%s \033[38;2;0;128;0m+0x%lx\n",
                i + 1,
                rip,
                symbol->name,
                offset);
        }
    }
}

} // namespace stacktrace
