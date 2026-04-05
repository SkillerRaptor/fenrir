/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "kernel/syscall/syscalls.hpp"

#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/core/logger.hpp"

namespace kernel::syscalls {

static constexpr u32 s_star_msr = 0xc0000081;
static constexpr u32 s_lstar_msr = 0xc0000082;
static constexpr u32 s_fmask_msr = 0xc0000084;

struct SyscallRegisters {
    u64 r15 { 0 };
    u64 r14 { 0 };
    u64 r13 { 0 };
    u64 r12 { 0 };
    u64 r11 { 0 };
    u64 r10 { 0 };
    u64 r9 { 0 };
    u64 r8 { 0 };

    u64 rsi { 0 };
    u64 rdi { 0 };
    u64 rbp { 0 };
    u64 rdx { 0 };
    u64 rcx { 0 };
    u64 rbx { 0 };
    u64 rax { 0 };
};

extern "C" void syscall_entry();

void initialize()
{
    // NOTE: Enable syscall instruction
    cpu::write_msr(0xc0000080, cpu::read_msr(0xc0000080) | (1 << 0));

    cpu::write_msr(s_star_msr, (static_cast<u64>(0x30 | 3) << 48) | (static_cast<u64>(0x28) << 32));

    cpu::write_msr(s_lstar_msr, reinterpret_cast<u64>(syscall_entry));

    // NOTE: Clear IF (interrupts) and DF (direction)
    cpu::write_msr(s_fmask_msr, (1 << 10) | (1 << 9));
}

extern "C" void syscall_handler(const SyscallRegisters *registers) { logger::info("Syscall: %u\n", registers->rax); }

} // namespace kernel::syscalls
