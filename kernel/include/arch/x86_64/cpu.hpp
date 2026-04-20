/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/gdt.hpp"
#include "scheduler/thread.hpp"

namespace cpu {

struct Core {
    u32 id = 0;
    u32 lapic_id = 0;
    u64 kernel_rsp = 0;
    u64 user_rsp = 0;

    gdt::CpuGdt gdt { };
    gdt::Tss tss { };

    u32 critical_sections = 0;
    bool were_interrupts_enabled = false;

    Thread *idle_thread = nullptr;
    Thread *current_thread = nullptr;
    Thread *next_thread = nullptr;
    usize thread_count = 0;
};

void early_initialize(u32 id, u32 lapic_id, u64 kernel_stack);
void initialize(u32 id);
void add_online();

void enter_critical();
void leave_critical();
bool is_in_critical();

Core &current();
Core &by_id(u32);
u32 online_count();

template <typename Fn>
static void for_each(const Fn &fn)
{
    for (usize i = 0; i < online_count(); ++i) {
        fn(by_id(i));
    }
}

inline void halt() { asm volatile("hlt"); }
inline void pause() { asm volatile("pause"); }

inline void enable_interrupts() { asm volatile("sti"); }
inline void disable_interrupts() { asm volatile("cli"); }

[[noreturn]] inline void hcf()
{
    while (true) {
        disable_interrupts();
        halt();
    }
}

inline u64 flags()
{
    volatile u64 flags = 0;
    asm volatile("pushf\n"
                 "pop %0\n"
                 : "=rm"(flags)
                 :
                 : "memory");
    return flags;
}

inline bool are_interrupts_enabled() { return (flags() & (1 << 9)) != 0; }

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

} // namespace cpu
