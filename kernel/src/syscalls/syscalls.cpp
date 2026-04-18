/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscall/syscalls.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"
#include "lib/string.hpp"
#include "syscall/syscalls/exit.hpp"

namespace syscalls {

static constexpr u32 s_star_msr = 0xc0000081;
static constexpr u32 s_lstar_msr = 0xc0000082;
static constexpr u32 s_fmask_msr = 0xc0000084;

using SyscallHandler = void (*)(const SyscallRegisters *);

extern "C" void syscall_entry();

static SyscallHandler s_syscall_handlers[256] { };

void initialize() { s_syscall_handlers[0x01] = sys$exit; }

void load()
{
    // NOTE: Enable syscall instruction
    cpu::write_msr(0xc0000080, cpu::read_msr(0xc0000080) | (1 << 0));

    cpu::write_msr(s_star_msr, (static_cast<u64>(0x30 | 3) << 48) | (static_cast<u64>(0x28) << 32));

    cpu::write_msr(s_lstar_msr, reinterpret_cast<u64>(syscall_entry));

    // NOTE: Clear IF (interrupts) and DF (direction)
    cpu::write_msr(s_fmask_msr, (1 << 10) | (1 << 9));
}

extern "C" void syscall_handler(const SyscallRegisters *registers)
{
    const u64 syscall_id = registers->rax;
    const u64 first_argument = registers->rdi;
    const u64 second_argument = registers->rsi;
    const u64 third_argument = registers->rdx;
    const u64 fourth_argument = registers->rcx;
    const u64 fifth_argument = registers->r8;

    if (s_syscall_handlers[syscall_id]) {
        s_syscall_handlers[syscall_id](registers);
        return;
    }

    switch (syscall_id) {
    case 0x02: {
        char *string = new char[second_argument];
        memcpy(string, reinterpret_cast<char *>(first_argument), second_argument);
        string[second_argument] = '\0';

        logger::info("%s\n", string);

        const limine_framebuffer *framebuffer = boot::get_framebuffers()[0];
        volatile u32 *fb_ptr = static_cast<volatile u32 *>(framebuffer->address);
        for (usize y = 0; y < framebuffer->height; y++) {
            for (usize x = 0; x < framebuffer->width; x++) {
                const u32 n_x = x * 255 / framebuffer->width;
                const u32 n_y = y * 255 / framebuffer->height;
                fb_ptr[y * (framebuffer->pitch / 4) + x] = (n_y << 8) | n_x;
            }
        }

        delete[] string;

        break;
    }
    default:
        logger::debug("Unhandled syscall %u\n", registers->rax);
        break;
    }
}

} // namespace syscalls
