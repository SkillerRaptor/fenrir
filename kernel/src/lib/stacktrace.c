/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "lib/stacktrace.h"

#include "boot/boot.h"
#include "lib/assertion.h"
#include "lib/log.h"
#include "lib/string.h"
#include "limine.h"
#include "mem/kmalloc.h"

struct StackFrame {
    struct StackFrame *rbp;
    uint64_t rip;
};

struct Symbol {
    uint64_t address;
    const char *name;
    size_t name_length;
};

static struct Symbol *s_symbols = nullptr;
static size_t s_symbol_count = 0;

static bool parse_hex(const char *text, uint64_t *out)
{
    uint64_t value = 0;
    for (size_t i = 0; i < 16; i++) {
        const char c = text[i];

        uint8_t nibble;
        if (c >= '0' && c <= '9') {
            nibble = (uint8_t) (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            nibble = (uint8_t) (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            nibble = (uint8_t) (c - 'A' + 10);
        } else {
            return false;
        }

        value = (value << 4) | nibble;
    }

    *out = value;

    return true;
}

static bool parse_line(const char *line, size_t length, struct Symbol *symbol)
{
    if (length > 0 && line[length - 1] == '\r') {
        length--;
    }

    if (length < 18 || line[16] != ' ') {
        return false;
    }

    uint64_t address = 0;
    if (!parse_hex(line, &address)) {
        return false;
    }

    symbol->address = address;
    symbol->name = line + 17;
    symbol->name_length = length - 17;

    return true;
}

static size_t parse_symbols(const char *text, size_t size, struct Symbol *symbols)
{
    size_t count = 0;

    size_t start = 0;
    while (start < size) {
        size_t end = start;
        while (end < size && text[end] != '\n') {
            ++end;
        }

        struct Symbol symbol = { };
        if (parse_line(text + start, end - start, &symbol)) {
            if (symbols != nullptr) {
                symbols[count] = symbol;
            }

            ++count;
        }

        start = end + 1;
    }

    return count;
}

void stacktrace_init()
{
    const struct limine_module_response *module_response = boot_get_module_response();
    const struct limine_file *symbol_map_file = module_response->modules[0];
    ASSERT(symbol_map_file != nullptr);

    const char *text = symbol_map_file->address;
    const size_t size = symbol_map_file->size;

    LOG_DEBUG("Stacktrace: Kernel symbol map found (%lu KiB)\n", size / 1024);

    const size_t count = parse_symbols(text, size, nullptr);
    VERIFY(count != 0);

    struct Symbol *symbols = kmalloc(count * sizeof(struct Symbol));
    VERIFY(symbols != nullptr);

    s_symbol_count = parse_symbols(text, size, symbols);
    s_symbols = symbols;

    LOG_DEBUG("Stacktrace: Loaded %lu symbols\n", s_symbol_count);

    LOG_INFO("Stacktrace: Initialized\n");
}

static const struct Symbol *find_symbol(uint64_t rip)
{
    size_t low = 0;
    size_t high = s_symbol_count;

    while (low < high) {
        const size_t mid = low + (high - low) / 2;
        if (s_symbols[mid].address <= rip) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    if (low == 0) {
        return nullptr;
    }

    return &s_symbols[low - 1];
}

void stacktrace_print(size_t max_frames)
{
    static constexpr char COLOR_RESET[] = "\x1b[0m";
    static constexpr char COLOR_ADDRESS[] = "\x1b[38;2;50;50;255m";
    static constexpr char COLOR_SYMBOL[] = "\x1b[38;2;255;215;0m";
    static constexpr char COLOR_OFFSET[] = "\x1b[38;2;0;128;0m";

    volatile struct StackFrame *stack_frame = nullptr;
    asm volatile("mov %%rbp, %0" : "=r"(stack_frame) : : "memory");

    const uint64_t hhdm_offset = boot_get_hhdm_offset();
    for (size_t i = 1; i < max_frames; ++i) {
        const uint64_t stack_frame_address = (uint64_t) stack_frame;
        if ((stack_frame_address & 0x7) != 0 || stack_frame_address < hhdm_offset) {
            break;
        }

        const uint64_t rip = stack_frame->rip;
        if (rip == 0) {
            break;
        }

        stack_frame = stack_frame->rbp;

        const struct Symbol *symbol = find_symbol(rip);
        if (symbol == nullptr) {
            LOG_ERROR(
                "  %02lu. %s%#018lx%s in %s??%s\n",
                i,
                COLOR_ADDRESS,
                rip,
                COLOR_RESET,
                COLOR_SYMBOL,
                COLOR_RESET);
            continue;
        }

        uint64_t offset = rip - symbol->address;
        if (offset == 0) {
            LOG_ERROR(
                "  %02lu. %s%#018lx%s in %s%.*s%s\n",
                i,
                COLOR_ADDRESS,
                rip,
                COLOR_RESET,
                COLOR_SYMBOL,
                (int) symbol->name_length,
                symbol->name,
                COLOR_RESET);
        } else {
            LOG_ERROR(
                "  %02lu. %s%#018lx%s in %s%.*s%s %s+%#lx%s\n",
                i,
                COLOR_ADDRESS,
                rip,
                COLOR_RESET,
                COLOR_SYMBOL,
                (int) symbol->name_length,
                symbol->name,
                COLOR_RESET,
                COLOR_OFFSET,
                offset,
                COLOR_RESET);
        }
    }
}
