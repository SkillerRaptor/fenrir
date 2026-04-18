/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#include "syscall/syscalls.hpp"

#include "arch/x86_64/cpu.hpp"
#include "core/boot.hpp"
#include "core/logger.hpp"

namespace syscalls {

static constexpr u32 s_star_msr = 0xc0000081;
static constexpr u32 s_lstar_msr = 0xc0000082;
static constexpr u32 s_fmask_msr = 0xc0000084;

using SyscallHandler = u64 (*)(Span<const u64>);

extern "C" void syscall_entry();

static SyscallHandler s_syscall_handlers[256] { };

void initialize()
{
    s_syscall_handlers[0x01] = sys$exit;
    s_syscall_handlers[0x02] = sys$enumerate_devices;
    s_syscall_handlers[0x03] = sys$open_device;
    s_syscall_handlers[0x04] = sys$close_device;
    s_syscall_handlers[0x05] = sys$map_device;
}

void load()
{
    // NOTE: Enable syscall instruction
    cpu::write_msr(0xc0000080, cpu::read_msr(0xc0000080) | (1 << 0));

    cpu::write_msr(s_star_msr, (static_cast<u64>(0x30 | 3) << 48) | (static_cast<u64>(0x28) << 32));

    cpu::write_msr(s_lstar_msr, reinterpret_cast<u64>(syscall_entry));

    // NOTE: Clear IF (interrupts) and DF (direction)
    cpu::write_msr(s_fmask_msr, (1 << 10) | (1 << 9));
}

extern "C" void syscall_handler(SyscallRegisters *registers)
{
    const u64 syscall_id = registers->rax;

    const SyscallHandler syscall_handler = s_syscall_handlers[syscall_id];
    if (!syscall_handler) {
        logger::err("Unknown syscall 0x%02x!\n", syscall_id);
        return;
    }

    registers->rax = syscall_handler({ registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8 });
}

} // namespace syscalls
