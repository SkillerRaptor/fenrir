/*
 * Copyright (c) 2026-present, SkillerRaptor
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "arch/x86_64/gdt.hpp"
#include "scheduler/thread.hpp"

class Cpu {
public:
    static void early_initialize(u32 id, u32 lapic_id, u64 kernel_stack);

    void initialize();

    u32 id() const { return m_id; }
    u32 lapic_id() const { return m_lapic_id; }

    // FIXME: Make this cleaner

    void set_current_thread(Thread *current_thread) { m_current_thread = current_thread; }
    Thread *current_thread() const { return m_current_thread; }

    void set_idle_thread(Thread *idle_thread) { m_idle_thread = idle_thread; }
    Thread *idle_thread() const { return m_idle_thread; }

    void set_next_thread(Thread *next_thread) { m_next_thread = next_thread; }
    Thread *next_thread() const { return m_next_thread; }

    void set_thread_count(const usize thread_count) { m_thread_count = thread_count; }
    usize thread_count() const { return m_thread_count; }

    void enter_critical_section();
    void leave_critical_section();
    bool is_in_critical_section() const;

    static Cpu &current();
    static Cpu &get_from_id(u32);
    static u32 online_count();

    static void halt() { asm volatile("hlt"); }
    static void pause() { asm volatile("pause"); }

    static void enable_interrupts() { asm volatile("sti"); }
    static void disable_interrupts() { asm volatile("cli"); }

    [[noreturn]] static void hcf()
    {
        while (true) {
            disable_interrupts();
            halt();
        }
    }

    static u64 flags()
    {
        volatile u64 flags = 0;
        asm volatile("pushf\n"
                     "pop %0\n"
                     : "=rm"(flags)
                     :
                     : "memory");
        return flags;
    }

    static bool are_interrupts_enabled() { return (flags() & (1 << 9)) != 0; }

    template <typename Fn>
    static void for_each(const Fn &fn)
    {
        for (usize i { 0 }; i < online_count(); ++i) {
            fn(get_from_id(i));
        }
    }

    // FIXME: This is a hack
    gdt::CpuGdt &gdt() { return m_gdt; }
    gdt::Tss &tss() { return m_tss; }

private:
    u32 m_id { 0 };
    u32 m_lapic_id { 0 };
    u64 m_kernel_rsp { 0 };
    u64 m_user_rsp { 0 };

    gdt::CpuGdt m_gdt { };
    gdt::Tss m_tss { };

    u32 m_critical_sections { 0 };
    bool m_were_interrupts_enabled { false };

    Thread *m_idle_thread { nullptr };
    Thread *m_current_thread { nullptr };
    Thread *m_next_thread { nullptr };
    usize m_thread_count { 0 };

    bool m_is_initialized { false };
};

namespace cpu {

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
