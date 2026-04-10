/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/gdt.hpp"
#include "scheduler/smp.hpp"
#include "scheduler/thread.hpp"

namespace cpu {

struct Info {
    u64 id { 0 };
    u64 user_rsp { 0 };
    u64 kernel_rsp { 0 };
    u32 lapic_id { 0 };

    gdt::CpuGdt gdt { };
    gdt::Tss tss { };

    Thread *idle_thread { nullptr };
    Thread *current_thread { nullptr };
    Thread *next_thread { nullptr };
};

inline void halt() { asm volatile("hlt"); }

inline void pause() { asm volatile("pause"); }

inline void enable_interrupts() { asm volatile("sti"); }

inline void disable_interrupts() { asm volatile("cli"); }

inline void write_msr(const u32 msr, const u64 value)
{
    const u32 high = (value >> 32) & 0xffffffff;
    const u32 low = (value >> 0) & 0xffffffff;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr) : "memory");
}

inline u64 read_msr(const u32 msr)
{
    u32 high = 0;
    u32 low = 0;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return (static_cast<u64>(high) << 32) | static_cast<u64>(low);
}

inline void set_fs_base(const void *address) { write_msr(0xc0000100, reinterpret_cast<u64>(address)); }
inline void set_gs_base(const void *address) { write_msr(0xc0000101, reinterpret_cast<u64>(address)); }
inline void set_kernel_gs_base(const void *address) { write_msr(0xc0000102, reinterpret_cast<u64>(address)); }

inline Info &get_local_cpu_info()
{
    u64 cpu_id = 0;
    asm volatile("mov %%gs:0x0, %0" : "=r"(cpu_id));
    return smp::get_cpu_infos()[cpu_id];
}

} // namespace cpu
